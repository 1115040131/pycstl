#pragma once

#include <fstream>
#include <string_view>

#include "tiny_db/row.h"

namespace tiny_db {

inline constexpr uint32_t kPageSize = 4096;
inline constexpr uint32_t kRowsPerPage = kPageSize / kRowSize;

inline constexpr uint32_t kTableMaxPages = 100;
inline constexpr uint32_t kTableMaxRows = kRowsPerPage * kTableMaxPages;

struct Pager {
    std::fstream file;
    std::streampos file_length;
    void* pages[kTableMaxPages] = {nullptr};

    Pager(std::string_view filename);

    void* GetPage(uint32_t page_num);

    void PageFlush(uint32_t page_num, uint32_t size);
};

}  // namespace tiny_db
