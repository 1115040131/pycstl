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

    struct iterator {
        using value_type = Row;
        using difference_type = ptrdiff_t;

    private:
        Table* table = nullptr;
        uint32_t row_num = 0;

        explicit iterator(Table* _table) noexcept : table(_table), row_num(0) {}
        iterator(Table* _table, uint32_t _row_num) noexcept : table(_table), row_num(_row_num) {}

        friend class Table;

    public:
        iterator() = default;

        iterator& operator++() noexcept {  //++iterator
            row_num++;
            return *this;
        }

        iterator operator++(int) noexcept {  // iterator++
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator--() noexcept {  //--iterator
            row_num--;
            return *this;
        }

        iterator operator--(int) noexcept {  // iterator--
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        Row& operator*() const {
            uint32_t page_num = row_num / kRowsPerPage;
            auto page = table->pager.GetPage(page_num);

            uint32_t row_offset = row_num % kRowsPerPage;
            uint32_t byte_offset = row_offset * kRowSize;
            return *reinterpret_cast<Row*>(static_cast<char*>(page) + byte_offset);
        }

        bool operator!=(const iterator& that) const noexcept {
            return table != that.table || row_num != that.row_num;
        }

        bool operator==(const iterator& that) const noexcept { return !(*this != that); }
    };

    struct const_iterator {
        using value_type = Row;
        using difference_type = ptrdiff_t;

    private:
        const Table* table = nullptr;
        uint32_t row_num = 0;

        explicit const_iterator(const Table* _table) noexcept : table(_table), row_num(0) {}
        const_iterator(const Table* _table, uint32_t _row_num) noexcept : table(_table), row_num(_row_num) {}

        friend class Table;

    public:
        const_iterator() = default;

        const_iterator(iterator that) noexcept : table(that.table), row_num(that.row_num) {}

        explicit operator iterator() const noexcept { return iterator{const_cast<Table*>(table)}; }

        const_iterator& operator++() noexcept {  //++iterator
            row_num++;
            return *this;
        }

        const_iterator operator++(int) noexcept {  // iterator++
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        const_iterator& operator--() noexcept {  //--iterator
            row_num--;
            return *this;
        }

        const_iterator operator--(int) noexcept {  // iterator--
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        const Row& operator*() const noexcept {
            uint32_t page_num = row_num / kRowsPerPage;
            auto page = const_cast<Table*>(table)->pager.GetPage(page_num);

            uint32_t row_offset = row_num % kRowsPerPage;
            uint32_t byte_offset = row_offset * kRowSize;
            return *reinterpret_cast<Row*>(static_cast<char*>(page) + byte_offset);
        }

        bool operator!=(const const_iterator& that) const noexcept {
            return table != that.table || row_num != that.row_num;
        }

        bool operator==(const const_iterator& that) const noexcept { return !(*this != that); }
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() { return iterator(this); }

    iterator end() { return iterator(this, num_rows); }

    const_iterator cbegin() const noexcept { return const_iterator(this); }

    const_iterator cend() const noexcept { return const_iterator(this, num_rows); }

    const_iterator begin() const noexcept { return cbegin(); }

    const_iterator end() const noexcept { return cend(); }

    reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }

    reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const noexcept { return std::make_reverse_iterator(cend()); }

    const_reverse_iterator crend() const noexcept { return std::make_reverse_iterator(cbegin()); }

    const_reverse_iterator rbegin() const noexcept { return crbegin(); }

    const_reverse_iterator rend() const noexcept { return crend(); }
};

}  // namespace tiny_db
