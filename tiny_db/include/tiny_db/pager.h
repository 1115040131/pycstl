#pragma once

#include <fstream>
#include <memory>
#include <string_view>

#include "tiny_db/defines.h"

namespace tiny_db {

struct Pager {
    friend class Table;

public:
    Pager(std::string_view filename);

    char* GetPage(uint32_t index);

    void PageFlush(uint32_t index);

private:
    std::fstream file_;
    std::streampos file_length_;
    std::unique_ptr<char[]> pages_[kTableMaxPages];
    uint32_t page_num_{0};
};

}  // namespace tiny_db
