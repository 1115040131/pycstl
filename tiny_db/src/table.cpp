#include "tiny_db/table.h"

#include <fmt/core.h>

namespace tiny_db {

Table::~Table() {
    uint32_t num_full_pages = num_rows / kRowsPerPage;

    if (pager.file.fail()) {
        fmt::print(stderr, "Error: file is in a bad state\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < num_full_pages; i++) {
        if (pager.pages[i] == nullptr) {
            continue;
        }
        pager.PageFlush(i, kPageSize);
        free(pager.pages[i]);
        pager.pages[i] = nullptr;
    }

    uint32_t num_additional_rows = num_rows % kRowsPerPage;
    if (num_additional_rows > 0) {
        uint32_t page_num = num_full_pages;
        if (pager.pages[page_num] != nullptr) {
            pager.PageFlush(page_num, num_additional_rows * kRowSize);
            free(pager.pages[page_num]);
            pager.pages[page_num] = nullptr;
        }
    }

    for (uint32_t i = 0; i < kTableMaxPages; i++) {
        if (pager.pages[i] != nullptr) {
            fmt::print(stderr, "Page {} is not nullptr\n", i);
            free(pager.pages[i]);
            pager.pages[i] = nullptr;
        }
    }
}

Row& Table::GetRow(uint32_t row_num) {
    uint32_t page_num = row_num / kRowsPerPage;
    auto page = pager.GetPage(page_num);

    uint32_t row_offset = row_num % kRowsPerPage;
    uint32_t byte_offset = row_offset * kRowSize;
    return *reinterpret_cast<Row*>(static_cast<char*>(page) + byte_offset);
}

}  // namespace tiny_db
