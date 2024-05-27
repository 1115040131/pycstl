#include "tiny_db/table.h"

#include <fmt/core.h>

namespace tiny_db {

Table::Table(std::string_view filename) : pager_(filename) {
    if (pager_.page_num_ == 0) {
        GetData(0).head.is_root = true;
    }
}

Table::~Table() {
    if (pager_.file_.fail()) {
        fmt::print(stderr, "Error: file is in a bad state\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < pager_.page_num_; i++) {
        if (pager_.pages_[i] == nullptr) {
            continue;
        }
        pager_.PageFlush(i);
        pager_.pages_[i].reset();
    }

    for (uint32_t i = 0; i < kTableMaxPages; i++) {
        if (pager_.pages_[i] != nullptr) {
            fmt::print(stderr, "Page {} is not nullptr\n", i);
            pager_.pages_[i].reset();
        }
    }
}

Table::iterator Table::Insert(const_iterator pos, const Row& value) {
    auto insert_pos = static_cast<iterator>(pos);
    insert_pos->key = value.id;
    insert_pos->value = value;
    GetData(insert_pos.page_index_).head.cell_num++;
    return insert_pos;
}

}  // namespace tiny_db
