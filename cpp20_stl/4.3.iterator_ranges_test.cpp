#include <algorithm>
#include <iterator>
#include <ranges>

#include <fmt/core.h>
#include <gtest/gtest.h>

namespace pyc {

template <typename T>
class Seq {
public:
    Seq(T start, T end) : start_{start}, end_{end} {}

    class iterator {
    public:
        // 兼容 clang
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = const T*;  // 迭代器不提供非 const 访问
        using reference = const T&;

        explicit iterator(T position = 0) : value_{position} {}

        reference operator*() const { return value_; }

        iterator& operator++() {
            ++value_;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& that) const noexcept { return value_ == that.value_; }
        bool operator!=(const iterator& that) const noexcept { return !(*this == that); }

    private:
        T value_;
    };

    iterator begin() const { return iterator{start_}; }  // 必须是 const 函数
    iterator end() const { return iterator{end_}; }      // 必须是 const 函数

private:
    T start_;
    T end_;
};

TEST(IteratorRangesTest, IteratorRangesTest) {
    constexpr int kStart = 100;
    constexpr int kEnd = 110;

    Seq<int> seq{kStart, kEnd};
    int curr = kStart;
    for (auto i : seq) {
        EXPECT_EQ(i, curr);
        curr++;
    }

    {
        auto [min_it, max_it] = std::minmax_element(seq.begin(), seq.end());
        EXPECT_EQ(*min_it, kStart);
        EXPECT_EQ(*max_it, kEnd - 1);
    }
    {
        auto [min_it, max_it] = std::ranges::minmax_element(seq);
        EXPECT_EQ(*min_it, kStart);
        EXPECT_EQ(*max_it, kEnd - 1);
    }
}

}  // namespace pyc
