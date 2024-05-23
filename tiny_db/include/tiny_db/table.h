#pragma once

#include <cstdint>
#include <string_view>

#include "tiny_db/pager.h"
#include "tiny_db/row.h"

namespace tiny_db {

struct Table {
    uint32_t num_rows = 0;
    Pager pager;

    Table(std::string_view filename) : pager(filename) { num_rows = pager.file_length / kRowSize; }
    ~Table();

    Row& GetRow(uint32_t row_num);
};

}  // namespace tiny_db
