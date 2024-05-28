#include "tiny_db/table.h"

#include <fmt/core.h>

namespace tiny_db {

Table::Table(std::string_view filename) : pager_(filename) {
    if (pager_.page_num_ == 0) {
        auto& root_page = GetData(0);
        root_page.head.type = DataType::Head::Type::kLeaf;
        root_page.head.is_root = true;
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
    auto& page_data = GetData(insert_pos.page_index_);

    for (uint32_t i = page_data.head.cell_num; i > insert_pos.cell_index_; i--) {
        std::swap(page_data.cells[i], page_data.cells[i - 1]);
    }

    insert_pos->key = value.id;
    insert_pos->value = value;
    page_data.head.cell_num++;
    return insert_pos;
}

Table::iterator Table::LowerBound(uint32_t key) {
    const auto& root_page = RootPage();
    if (root_page.head.type == DataType::Head::Type::kLeaf) {
        auto iter = std::lower_bound(root_page.cells.begin(), root_page.cells.begin() + root_page.head.cell_num,
                                     key, [](const auto& cell, uint32_t key) { return cell.key < key; });
        return iterator(this, root_page_index_, iter - root_page.cells.begin());
    } else {
        fmt::println("Need to implement searching internal nodes.");
        exit(EXIT_FAILURE);
    }
    return end();
}

}  // namespace tiny_db
