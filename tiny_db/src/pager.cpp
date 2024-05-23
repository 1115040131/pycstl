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

Pager::Pager(std::string_view filename) {
    if (std::filesystem::exists(filename)) {
        file.open(filename.data(), std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    } else {
        file.open(filename.data(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    }

    if (!file.is_open()) {
        fmt::print(stderr, "Error: cannot open file: \"{}\" - {}\n", filename, StreamStateToString(file));
        exit(EXIT_FAILURE);
    }

    if (file.fail()) {
        fmt::print(stderr, "Error: file is in a bad state\n");
        exit(EXIT_FAILURE);
    }

    file_length = file.tellg();
}

void* Pager::GetPage(uint32_t page_num) {
    if (page_num > kTableMaxPages) {
        fmt::println("Tried to fetch page number out of bounds. {} > {}", page_num, kTableMaxPages);
        exit(EXIT_FAILURE);
    }

    if (pages[page_num] == nullptr) {
        // Cache missed
        void* page = malloc(kPageSize);

        // 读取完整页面
        uint32_t full_page_num = file_length / kPageSize;
        if (page_num < full_page_num) {
            file.seekg(page_num * kPageSize, std::ios::beg);
            if (file.fail()) {
                fmt::print(stderr, "Error: seekg page_num: {} failed: {}\n", page_num, StreamStateToString(file));
                exit(EXIT_FAILURE);
            }

            file.read(static_cast<char*>(page), kPageSize);
            if (file.fail()) {
                fmt::print(stderr, "Error: read page_num: {} failed: {}\n", page_num, StreamStateToString(file));
                exit(EXIT_FAILURE);
            }
        }

        // 读取不完整页面
        if ((file_length % kPageSize) && page_num == full_page_num) {
            file.seekg(page_num * kPageSize, std::ios::beg);
            if (file.fail()) {
                fmt::print(stderr, "Error: seekg page_num: {} failed: {}\n", page_num, StreamStateToString(file));
                exit(EXIT_FAILURE);
            }

            file.read(static_cast<char*>(page), file_length % kPageSize);
            if (file.fail()) {
                fmt::print(stderr, "Error: read page_num: {} failed: {}\n", page_num, StreamStateToString(file));
                exit(EXIT_FAILURE);
            }
        }

        pages[page_num] = page;
    }

    return pages[page_num];
}

void Pager::PageFlush(uint32_t page_num, uint32_t size) {
    auto page = pages[page_num];

    if (page == nullptr) {
        fmt::print(stderr, "Tried to flush null page {}\n", page_num);
        exit(EXIT_FAILURE);
    }

    file.seekp(page_num * kPageSize, std::ios::beg);
    if (file.fail()) {
        fmt::print(stderr, "Error: seekp: {} failed: {}\n", page_num * kPageSize, StreamStateToString(file));
        exit(EXIT_FAILURE);
    }

    file.write(static_cast<char*>(page), size);
    if (file.fail()) {
        fmt::print(stderr, "Error: write page_num: {} failed: {}\n", page_num, StreamStateToString(file));
        exit(EXIT_FAILURE);
    }
}

}  // namespace tiny_db
