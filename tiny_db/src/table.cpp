#include "tiny_db/table.h"

#include <algorithm>

#include <fmt/core.h>

namespace tiny_db {

Table::Table(std::string_view filename) : pager_(filename) {
    if (pager_.page_num_ == 0) {
        auto& root_node = GetLeafNode(0);
        root_node.type = Node::Type::kLeaf;
    } else {
        // TODO: 单独存储根节点, 首尾叶节点信息
        while (true) {
            const auto& root_node = GetNode(root_page_index_);
            if (root_node.parent != 0) {
                root_page_index_ = root_node.parent;
            } else {
                break;
            }
        }

        first_leaf_page_index_ = root_page_index_;
        while (true) {
            const auto& first_leaf = GetNode(first_leaf_page_index_);
            if (first_leaf.type == Node::Type::kLeaf) {
                break;
            } else {
                first_leaf_page_index_ =
                    reinterpret_cast<const InternalNodeType&>(first_leaf).children[0].page_index;
            }
        }

        last_leaf_page_index_ = root_page_index_;
        while (true) {
            const auto& last_leaf = GetNode(last_leaf_page_index_);
            if (last_leaf.type == Node::Type::kLeaf) {
                break;
            } else {
                last_leaf_page_index_ = reinterpret_cast<const InternalNodeType&>(last_leaf).right_child;
            }
        }
    }
}

Table::~Table() {
    if (pager_.file_.fail()) {
        fmt::print(stderr, "Error: file is in a bad state\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < pager_.page_num_; i++) {
        if (pager_.pages_[i] == nullptr) {
            continue;
        }
        pager_.PageFlush(i);
        pager_.pages_[i].reset();
    }

    for (uint32_t i = 0; i < kTableMaxPages; i++) {
        if (pager_.pages_[i] != nullptr) {
            fmt::print(stderr, "Page {} is not nullptr\n", i);
            pager_.pages_[i].reset();
        }
    }
}

Table::iterator Table::Insert(const_iterator pos, const Row& value) {
    auto insert_pos = static_cast<iterator>(pos);
    auto& page_data = GetLeafNode(insert_pos.page_index_);

    for (uint32_t i = page_data.cell_num; i > insert_pos.cell_index_; i--) {
        std::swap(page_data.cells[i], page_data.cells[i - 1]);
    }

    insert_pos->key = value.id;
    insert_pos->value = value;
    page_data.cell_num++;
    return insert_pos;
}

void Table::SplitAndInsert(const_iterator pos, const Row& value) {
    uint32_t old_node_page_index = pos.page_index_;
    auto& old_node = GetLeafNode(old_node_page_index);

    // 拆分边界
    constexpr uint32_t kSplitPoint = (LeafNodeType::kMaxCells + 1) / 2;

    // cells 拷贝到新的右子节点
    uint32_t right_child_page_index = pager_.page_num_;
    auto& right_child = GetLeafNode(right_child_page_index);
    right_child = LeafNodeType();
    right_child.page_index = right_child_page_index;

    uint32_t move_count = pos.cell_index_ < kSplitPoint ? kSplitPoint : kSplitPoint - 1;
    for (uint32_t i = 0; i < move_count; i++) {
        std::swap(old_node.cells[LeafNodeType::kMaxCells - move_count + i], right_child.cells[i]);
    }
    old_node.cell_num = LeafNodeType::kMaxCells - move_count;
    right_child.cell_num = move_count;

    // 插入新数据
    auto insert_pos = pos.cell_index_ < kSplitPoint
                          ? pos
                          : iterator(this, right_child_page_index, pos.cell_index_ - kSplitPoint);
    Insert(insert_pos, value);

    // 更新兄弟节点
    right_child.next_leaf = old_node.next_leaf;
    old_node.next_leaf = right_child_page_index;

    // 更新父节点
    if (old_node.parent) {
        auto& parent = GetInternalNode(old_node.parent);
        if (parent.right_child == old_node_page_index) {
            parent.children[parent.child_num] = {old_node_page_index, old_node.cells[old_node.cell_num - 1].key};
            parent.right_child = right_child_page_index;
            parent.child_num++;
        } else {
            auto iter = std::find_if(
                parent.children.begin(), parent.children.begin() + parent.child_num,
                [old_node_page_index](const auto& child) { return child.page_index == old_node_page_index; });
            if (iter == parent.children.begin() + parent.child_num) {
                fmt::print(stderr, "Error: cannot find old node {} in parent\n", old_node_page_index);
                exit(EXIT_FAILURE);
            }

            // 插入新的子节点
            iter->max_key = old_node.cells[old_node.cell_num - 1].key;
            for (uint32_t i = parent.child_num; i > iter - parent.children.begin() + 1; i--) {
                std::swap(parent.children[i], parent.children[i - 1]);
            }
            *(++iter) = {right_child_page_index, right_child.cells[right_child.cell_num - 1].key};
            parent.child_num++;
        }

        right_child.parent = old_node.parent;
        if (last_leaf_page_index_ == old_node_page_index) {
            last_leaf_page_index_ = right_child_page_index;
        }

        if (parent.child_num > InternalNodeType::kMaxChildren) {
            split_internal_node(parent);
        }
    } else {
        // 创建新的根节点
        int new_root_page_index = pager_.page_num_;
        auto& new_root = GetInternalNode(new_root_page_index);
        new_root.type = Node::Type::kInternal;
        new_root.page_index = new_root_page_index;
        new_root.child_num = 1;
        new_root.children[0].page_index = pos.page_index_;
        new_root.children[0].max_key = old_node.cells[old_node.cell_num - 1].key;
        new_root.right_child = right_child_page_index;

        old_node.parent = new_root_page_index;
        right_child.parent = new_root_page_index;

        root_page_index_ = new_root_page_index;
        first_leaf_page_index_ = pos.page_index_;
        last_leaf_page_index_ = right_child_page_index;
    }
}

void Table::split_internal_node(InternalNodeType& old_node) {
    // 拆分边界
    // right_child + 1, 叶子节点拆分 + 1
    constexpr uint32_t kTotalNode = InternalNodeType::kMaxChildren + 2;
    constexpr uint32_t kSplitPoint = kTotalNode / 2;
    // constexpr uint32_t kSplitPoint = (kTotalNode + 1) / 2;
    constexpr uint32_t kMoveCount = kTotalNode - kSplitPoint;

    // children 拷贝到新的右子节点
    uint32_t new_node_page_index = pager_.page_num_;
    auto& new_node = GetInternalNode(new_node_page_index);
    new_node = InternalNodeType();
    new_node.page_index = new_node_page_index;

    for (uint32_t i = 0; i < kMoveCount - 1; i++) {
        new_node.children[i] = old_node.children[kSplitPoint + i];
    }
    new_node.child_num = kMoveCount - 1;
    new_node.right_child = old_node.right_child;
    GetLeafNode(new_node.right_child).parent = new_node_page_index;

    old_node.child_num = kSplitPoint - 1;
    old_node.right_child = old_node.children[kSplitPoint - 1].page_index;

    // 更新父节点
    if (old_node.parent) {
        // TODO: 递归拆分 internal 节点
        fmt::print(stderr, "Need to implement splitting internal node recursively.\n");
        exit(EXIT_FAILURE);
    } else {
        // 创建新的根节点
        int new_root_page_index = pager_.page_num_;
        auto& new_root = GetInternalNode(new_root_page_index);
        new_root.type = Node::Type::kInternal;
        new_root.page_index = new_root_page_index;
        new_root.child_num = 1;
        new_root.children[0].page_index = root_page_index_;
        auto& right_child = GetLeafNode(old_node.right_child);
        new_root.children[0].max_key = right_child.cells[right_child.cell_num - 1].key;
        new_root.right_child = new_node_page_index;

        old_node.parent = new_root_page_index;
        new_node.parent = new_root_page_index;

        root_page_index_ = new_root_page_index;
    }
}

Table::iterator Table::lower_bound(uint32_t key, uint32_t page_index) {
    const auto& page = GetNode(page_index);
    switch (page.type) {
        case Node::Type::kInternal: {
            const auto& internal_page = reinterpret_cast<const InternalNodeType&>(page);
            auto iter = std::lower_bound(internal_page.children.begin(),
                                         internal_page.children.begin() + internal_page.child_num, key,
                                         [](const auto& child, uint32_t key) { return child.max_key < key; });
            if (iter == internal_page.children.begin() + internal_page.child_num) {
                return lower_bound(key, internal_page.right_child);
            } else {
                return lower_bound(key, iter->page_index);
            }
        }
        case Node::Type::kLeaf: {
            const auto& leaf_page = reinterpret_cast<const LeafNodeType&>(page);
            auto iter = std::lower_bound(leaf_page.cells.begin(), leaf_page.cells.begin() + leaf_page.cell_num,
                                         key, [](const auto& cell, uint32_t key) { return cell.key < key; });
            return iterator(this, page_index, iter - leaf_page.cells.begin());
        }
        default:
            fmt::println("Unknown node type");
            exit(EXIT_FAILURE);
            break;
    }
}

void indent(uint32_t level) {
    for (uint32_t i = 0; i < level; i++) {
        fmt::print("  ");
    }
}

void Table::print_tree(uint32_t page_index, uint32_t indentation_level, bool debug) const {
    switch (GetNode(page_index).type) {
        case Node::Type::kInternal: {
            const auto& node = GetInternalNode(page_index);
            indent(indentation_level);
            if (debug) {
                fmt::println("- internal (size {} page_index {})", node.child_num, node.page_index);
            } else {
                fmt::println("- internal (size {})", node.child_num);
            }
            for (uint32_t i = 0; i < node.child_num; i++) {
                print_tree(node.children[i].page_index, indentation_level + 1, debug);

                indent(indentation_level + 1);
                fmt::println("- key {}", node.children[i].max_key);
            }
            print_tree(node.right_child, indentation_level + 1, debug);
            break;
        }
        case Node::Type::kLeaf: {
            const auto& node = GetLeafNode(page_index);
            indent(indentation_level);
            if (debug) {
                fmt::println("- leaf (size {} page_index {})", node.cell_num, node.page_index);
            } else {
                fmt::println("- leaf (size {})", node.cell_num);
            }
            for (uint32_t i = 0; i < node.cell_num; i++) {
                indent(indentation_level + 1);
                fmt::println("- {}", node.cells[i].key);
            }
            break;
        }
        default:
            fmt::print(stderr, "Error: unknown node type\n");
            exit(EXIT_FAILURE);
    }
}

}  // namespace tiny_db
