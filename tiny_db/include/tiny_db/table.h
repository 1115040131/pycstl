#pragma once

#include <cstdint>
#include <string_view>

#include "tiny_db/node.h"
#include "tiny_db/pager.h"
#include "tiny_db/row.h"

namespace tiny_db {

class Table {
public:
    using LeafNodeType = LeafNode<uint32_t, Row>;
    using InternalNodeType = InternalNode<uint32_t>;

    Table(std::string_view filename);
    ~Table();

    inline Node& GetNode(uint32_t page_index) { return *reinterpret_cast<Node*>(pager_.GetPage(page_index)); }

    inline const Node& GetNode(uint32_t page_index) const { return const_cast<Table*>(this)->GetNode(page_index); }

    inline LeafNodeType& GetLeafNode(uint32_t page_index) {
        return *reinterpret_cast<LeafNodeType*>(pager_.GetPage(page_index));
    }

    inline const LeafNodeType& GetLeafNode(uint32_t page_index) const {
        return const_cast<Table*>(this)->GetLeafNode(page_index);
    }

    inline InternalNodeType& GetInternalNode(uint32_t page_index) {
        return *reinterpret_cast<InternalNodeType*>(pager_.GetPage(page_index));
    }

    inline const InternalNodeType& GetInternalNode(uint32_t page_index) const {
        return const_cast<Table*>(this)->GetInternalNode(page_index);
    }

    inline uint32_t GetPageNum() const noexcept { return pager_.page_num_; }

    struct iterator {
        using value_type = Row;
        using difference_type = ptrdiff_t;

    private:
        explicit iterator(Table* table) noexcept : table_(table) {}
        iterator(Table* table, uint32_t page_index) noexcept : table_(table), page_index_(page_index) {
            cell_index_ = table_->GetLeafNode(page_index).cell_num;
            if (cell_index_ == LeafNodeType::kMaxCells) {
                cell_index_ = 0;
                page_index_++;
            }
        }

        iterator(Table* table, uint32_t page_index, uint32_t cell_index) noexcept
            : table_(table), page_index_(page_index), cell_index_(cell_index) {}

        friend class Table;

    public:
        iterator() = default;

        iterator& operator++() noexcept {  //++iterator
            if (cell_index_ + 1 < LeafNodeType::kMaxCells) {
                cell_index_++;
            } else {
                cell_index_ = 0;
                page_index_++;
            }
            return *this;
        }

        iterator operator++(int) noexcept {  // iterator++
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator--() noexcept {  //--iterator
            if (cell_index_ > 0) {
                cell_index_--;
            } else {
                cell_index_ = LeafNodeType::kMaxCells - 1;
                page_index_--;
            }
            return *this;
        }

        iterator operator--(int) noexcept {  // iterator--
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        LeafNodeType::Cell& operator*() const { return table_->GetLeafNode(page_index_).cells[cell_index_]; }

        LeafNodeType::Cell* operator->() const { return &table_->GetLeafNode(page_index_).cells[cell_index_]; }

        bool operator!=(const iterator& that) const noexcept {
            return table_ != that.table_ || page_index_ != that.page_index_ || cell_index_ != that.cell_index_;
        }

        bool operator==(const iterator& that) const noexcept { return !(*this != that); }

    private:
        Table* table_{nullptr};
        uint32_t page_index_{0};
        uint32_t cell_index_{0};
    };

    struct const_iterator {
        using value_type = Row;
        using difference_type = ptrdiff_t;

    private:
        explicit const_iterator(const Table* table) noexcept : table_(table) {}
        const_iterator(const Table* table, uint32_t page_index) noexcept : table_(table), page_index_(page_index) {
            cell_index_ = table_->GetLeafNode(page_index).cell_num;
            if (cell_index_ == LeafNodeType::kMaxCells) {
                cell_index_ = 0;
                page_index_++;
            }
        }

        const_iterator(const Table* table, uint32_t page_index, uint32_t cell_index) noexcept
            : table_(table), page_index_(page_index), cell_index_(cell_index) {}

        friend class Table;

    public:
        const_iterator() = default;

        const_iterator(iterator that) noexcept
            : table_(that.table_), page_index_(that.page_index_), cell_index_(that.cell_index_) {}

        explicit operator iterator() const noexcept {
            return iterator{const_cast<Table*>(table_), page_index_, cell_index_};
        }

        const_iterator& operator++() noexcept {  //++iterator
            if (cell_index_ + 1 < LeafNodeType::kMaxCells) {
                cell_index_++;
            } else {
                cell_index_ = 0;
                page_index_++;
            }
            return *this;
        }

        const_iterator operator++(int) noexcept {  // iterator++
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        const_iterator& operator--() noexcept {  //--iterator
            if (cell_index_ > 0) {
                cell_index_--;
            } else {
                cell_index_ = LeafNodeType::kMaxCells - 1;
                page_index_--;
            }
            return *this;
        }

        const_iterator operator--(int) noexcept {  // iterator--
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        const LeafNodeType::Cell& operator*() const { return table_->GetLeafNode(page_index_).cells[cell_index_]; }

        const LeafNodeType::Cell* operator->() const {
            return &table_->GetLeafNode(page_index_).cells[cell_index_];
        }

        bool operator!=(const iterator& that) const noexcept {
            return table_ != that.table_ || page_index_ != that.page_index_ || cell_index_ != that.cell_index_;
        }

        bool operator==(const const_iterator& that) const noexcept { return !(*this != that); }

    private:
        const Table* table_{nullptr};
        uint32_t page_index_{0};
        uint32_t cell_index_{0};
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

#pragma region 迭代器

    iterator begin() { return iterator(this); }

    iterator end() { return iterator(this, pager_.page_num_ - 1); }

    const_iterator cbegin() const noexcept { return const_iterator(this); }

    const_iterator cend() const noexcept { return const_iterator(this, pager_.page_num_ - 1); }

    const_iterator begin() const noexcept { return cbegin(); }

    const_iterator end() const noexcept { return cend(); }

    reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }

    reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const noexcept { return std::make_reverse_iterator(cend()); }

    const_reverse_iterator crend() const noexcept { return std::make_reverse_iterator(cbegin()); }

    const_reverse_iterator rbegin() const noexcept { return crbegin(); }

    const_reverse_iterator rend() const noexcept { return crend(); }

#pragma endregion

#pragma region 元素访问

    inline Node& RootPage() { return GetNode(root_page_index_); }
    inline const Node& RootPage() const { return GetNode(root_page_index_); }

    // inline DataType& FrontPage() { return GetData(0); }
    // inline const DataType& FrontPage() const { return GetData(0); }

    // inline DataType& BackPage() { return GetData(GetPageNum() - 1); }
    // inline const DataType& BackPage() const { return GetData(GetPageNum() - 1); }

#pragma endregion

#pragma region 容量

    // bool Empty() const { return RootPage().head.cell_num == 0; }

    bool Full() const { return GetLeafNode(root_page_index_).cell_num == LeafNodeType::kMaxCells; }

#pragma endregion

#pragma region 修改器

    iterator Insert(const_iterator pos, const Row& value);

    void Split(const_iterator pos);

#pragma endregion

#pragma region 查找

    // iterator Find(uint32_t key);

    iterator LowerBound(uint32_t key);

#pragma endregion

#pragma region 观察者

    void PrintTree() const { PrintNode(root_page_index_, 0); }

#pragma endregion

private:
    void PrintNode(uint32_t page_index, uint32_t indentation_level) const;

private:
    uint32_t root_page_index_ = 0;
    Pager pager_;
};

}  // namespace tiny_db
