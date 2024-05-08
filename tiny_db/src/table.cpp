#include "tiny_db/table.h"

namespace tiny_db {

Table::~Table() {
    for (auto page : pages) {
        if (page) {
            free(page);
        }
    }
}

Row& Table::GetRow(uint32_t row_num) {
    uint32_t page_num = row_num / kRowsPerPage;
    auto& page = pages[page_num];
    if (!page) {
        page = malloc(kPageSize);
    }
    uint32_t row_offset = row_num % kRowsPerPage;
    uint32_t byte_offset = row_offset * kRowSize;
    return *reinterpret_cast<Row*>(static_cast<char*>(page) + byte_offset);
}

}  // namespace tiny_db
