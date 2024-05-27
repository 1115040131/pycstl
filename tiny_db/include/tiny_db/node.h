#pragma once

#include <array>
#include <cstdint>

#include <fmt/core.h>

#include "tiny_db/defines.h"

namespace tiny_db {

template <typename Key, typename Value>
struct Node {
    struct Head {
        enum class Type {
            kInternal,
            kLeaf,
        };

        Type type{Type::kLeaf};
        bool is_root{false};
        Node* parent{nullptr};
        uint32_t cell_num{0};
    };

    struct Cell {
        Key key;
        Value value;
    };

    // 计算可以存储的最大cell数量
    static constexpr uint32_t kHeadSize = sizeof(Head);

    static constexpr uint32_t kKeySize = sizeof(Key);
    static constexpr uint32_t kValueSize = sizeof(Value);
    static constexpr uint32_t kCellSize = kKeySize + kValueSize;

    static constexpr uint32_t kMaxCells = (kPageSize - kHeadSize) / kCellSize;

    Head head;
    std::array<Cell, kMaxCells> cells;

    void Print() const {
        fmt::println("leaf (size {})", head.cell_num);
        for (uint32_t i = 0; i < head.cell_num; i++) {
            fmt::println("  - {} : {}", i, cells[i].key);
        }
    }
};

}  // namespace tiny_db
