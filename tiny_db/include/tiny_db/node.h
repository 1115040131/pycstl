#pragma once

#include <array>
#include <cstdint>

#include <fmt/core.h>

#include "tiny_db/defines.h"

namespace tiny_db {

struct Node {
    enum class Type {
        kInternal,
        kLeaf,
    };
    Type type{Type::kInternal};
    bool is_root{false};
    Node* parent{nullptr};
};

template <typename Key, typename Value>
struct LeafNode : public Node {
    struct Cell {
        Key key;
        Value value;
    };

    uint32_t cell_num{0};

    // 计算可以存储的最大cell数量
    static constexpr uint32_t kHeadSize = sizeof(Node) + sizeof(uint32_t);
    static constexpr uint32_t kCellSize = sizeof(Cell);
    static constexpr uint32_t kMaxCells = (kPageSize - kHeadSize) / kCellSize;

    std::array<Cell, kMaxCells> cells;

    explicit LeafNode() : Node(Node::Type::kLeaf) {}
    explicit LeafNode(Node* node) : Node(Node::Type::kLeaf, false, node) {}

public:
    void Print() const {
        fmt::println("leaf (size {})", cell_num);
        for (uint32_t i = 0; i < cell_num; i++) {
            fmt::println("  - {} : {}", i, cells[i].key);
        }
    }
};

template <typename Key>
struct InternalNode : public Node {
    struct Child {
        uint32_t page_index;
        Key max_key;
    };

    uint32_t child_num{0};
    uint32_t right_child{0};

    // 计算可以存储的最大 child 数量
    static constexpr uint32_t kHeadSize = sizeof(Node) + sizeof(child_num) + sizeof(right_child);
    static constexpr uint32_t kChildNum = sizeof(Child);
    static constexpr uint32_t kMaxChildren = (kPageSize - kHeadSize) / kChildNum;

    std::array<Child, kMaxChildren> children;

    InternalNode() : Node(Node::Type::kInternal) {}
    InternalNode(Node* node) : Node(Node::Type::kInternal, false, node) {}
};

}  // namespace tiny_db
