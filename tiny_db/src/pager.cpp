#include "tiny_db/pager.h"

#include <filesystem>

#include <fmt/core.h>

namespace tiny_db {

std::string StreamStateToString(std::ios& stream) {
    if (stream.bad()) {
        return "Badbit: A serious error occurred on the associated stream.";
    } else if (stream.fail()) {
        return "Failbit: Logical error on i/o operation.";
    } else if (stream.eof()) {
        return "Eofbit: End-of-File reached on input operation.";
    } else {
        return "No error: Operation successful.";
    }
}

Pager::Pager(std::string_view file_name) {
    if (std::filesystem::exists(file_name)) {
        file_.open(file_name.data(), std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    } else {
        file_.open(file_name.data(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    }

    if (!file_.is_open()) {
        fmt::print(stderr, "Error: cannot open file_: \"{}\" - {}\n", file_name, StreamStateToString(file_));
        exit(EXIT_FAILURE);
    }

    if (file_.fail()) {
        fmt::print(stderr, "Error: file_ is in a bad state\n");
        exit(EXIT_FAILURE);
    }

    file_length_ = file_.tellg();

    if (file_length_ % kPageSize != 0) {
        fmt::print(stderr, "Db file_ is not a whole number of pages_. Corrupt file_.\n");
        exit(EXIT_FAILURE);
    }

    page_num_ = file_length_ / kPageSize;
}

char* Pager::GetPage(uint32_t index) {
    if (index > kTableMaxPages) {
        fmt::println("Tried to fetch page number out of bounds. {} > {}", index, kTableMaxPages);
        exit(EXIT_FAILURE);
    }

    if (pages_[index] == nullptr) {
        // Cache missed
        char* page = new char[kPageSize];

        // 读取完整页面
        if (index < page_num_) {
            file_.seekg(index * kPageSize, std::ios::beg);
            if (file_.fail()) {
                fmt::print(stderr, "Error: seekg page index: {} failed: {}\n", index, StreamStateToString(file_));
                exit(EXIT_FAILURE);
            }

            file_.read(page, kPageSize);
            if (file_.fail()) {
                fmt::print(stderr, "Error: read page index: {} failed: {}\n", index, StreamStateToString(file_));
                exit(EXIT_FAILURE);
            }
        }

        pages_[index].reset(page);

        if (index >= page_num_) {
            page_num_ = index + 1;
        }
    }

    return pages_[index].get();
}

void Pager::PageFlush(uint32_t index) {
    auto page = pages_[index].get();

    if (page == nullptr) {
        fmt::print(stderr, "Tried to flush null page {}\n", index);
        exit(EXIT_FAILURE);
    }

    file_.seekp(index * kPageSize, std::ios::beg);
    if (file_.fail()) {
        fmt::print(stderr, "Error: seekp: {} failed: {}\n", index * kPageSize, StreamStateToString(file_));
        exit(EXIT_FAILURE);
    }

    file_.write(static_cast<char*>(page), kPageSize);
    if (file_.fail()) {
        fmt::print(stderr, "Error: write page index: {} failed: {}\n", index, StreamStateToString(file_));
        exit(EXIT_FAILURE);
    }
}

}  // namespace tiny_db
