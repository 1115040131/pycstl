#pragma once

#include <cstdint>

#include "tiny_db/row.h"

namespace tiny_db {

inline constexpr uint32_t kPageSize = 4096;
inline constexpr uint32_t kRowsPerPage = kPageSize / kRowSize;

inline constexpr uint32_t kTableMaxPages = 100;
inline constexpr uint32_t kTableMaxRows = kRowsPerPage * kTableMaxPages;

struct Table {
    uint32_t num_rows = 0;
    void* pages[kTableMaxPages] = {nullptr};

    Table() = default;
    ~Table();

    Row& GetRow(uint32_t row_num);
};

}  // namespace tiny_db
