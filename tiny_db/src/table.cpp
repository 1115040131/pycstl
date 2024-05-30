#include "tiny_db/table.h"

#include <fmt/core.h>

namespace tiny_db {

Table::Table(std::string_view filename) : pager_(filename) {
    if (pager_.page_num_ == 0) {
        auto& root_node = GetLeafNode(root_page_index_);
        root_node.type = Node::Type::kLeaf;
        root_node.is_root = true;
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

void Table::Split(const_iterator pos) {
    auto& parent = GetNode(pos.page_index_);

    // TODO: 暂不实现更新拆分后的父节点
    if (parent.parent) {
        fmt::print(stderr, "Need to implement updating parent after split.\n");
        exit(EXIT_FAILURE);
    }

    Node old_head = parent;  // 保存旧的头部信息

    constexpr uint32_t kSplitPoint = (LeafNodeType::kMaxCells + 1) / 2;

    // cells 拷贝到新的左子节点
    int left_child_page_index = pager_.page_num_;
    auto& left_child = GetLeafNode(left_child_page_index);
    left_child = LeafNodeType(&parent);

    for (uint32_t i = 0; i < kSplitPoint; i++) {
        left_child.cells[i] = std::move(reinterpret_cast<LeafNodeType&>(parent).cells[i]);
        left_child.cell_num++;
    }

    // cells 拷贝到新的右子节点
    int right_child_page_index = pager_.page_num_;
    auto& right_child = GetLeafNode(right_child_page_index);
    right_child = LeafNodeType(&parent);

    for (uint32_t i = kSplitPoint; i < LeafNodeType::kMaxCells; i++) {
        right_child.cells[i - kSplitPoint] = std::move(reinterpret_cast<LeafNodeType&>(parent).cells[i]);
        right_child.cell_num++;
    }

    left_child.next_leaf = right_child_page_index;

    // 更新父节点
    auto& new_parent = reinterpret_cast<InternalNodeType&>(parent);
    new_parent = InternalNodeType{};
    new_parent.type = Node::Type::kInternal;
    new_parent.is_root = old_head.is_root;
    new_parent.parent = old_head.parent;

    new_parent.child_num = 1;
    new_parent.children[0].page_index = left_child_page_index;
    new_parent.children[0].max_key = left_child.cells[left_child.cell_num - 1].key;
    new_parent.right_child = right_child_page_index;

    // 更新头尾叶子节点
    if (first_leaf_page_index_ == pos.page_index_) {
        first_leaf_page_index_ = left_child_page_index;
    }
    if (last_leaf_page_index_ == pos.page_index_) {
        last_leaf_page_index_ = right_child_page_index;
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

void Table::print_tree(uint32_t page_index, uint32_t indentation_level) const {
    switch (GetNode(page_index).type) {
        case Node::Type::kInternal: {
            const auto& node = GetInternalNode(page_index);
            indent(indentation_level);
            fmt::println("- internal (size {})", node.child_num);
            for (uint32_t i = 0; i < node.child_num; i++) {
                print_tree(node.children[i].page_index, indentation_level + 1);

                indent(indentation_level + 1);
                fmt::println("- key {}", node.children[i].max_key);
            }
            print_tree(node.right_child, indentation_level + 1);
            break;
        }
        case Node::Type::kLeaf: {
            const auto& node = GetLeafNode(page_index);
            indent(indentation_level);
            fmt::println("- leaf (size {})", node.cell_num);
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
