#include <algorithm>
#include <ranges>
#include <vector>

#include <fmt/core.h>
#include <gtest/gtest.h>

namespace pyc {

// 范围：是一个可迭代对象的集合, 支持 begin() 和 end() 迭代器的结构都是范围
// 视图：是转换另一个基础范围的范围. 视图是惰性的, 只在范围迭代时操作. 视图从底层范围返回数据, 不拥有任何数据.
// 视图的运行时间复杂度是 O(1)
// 视图适配器是一个对象, 可接受一个范围, 并返回一个视图对象. 试图适配器可以使用 | 连接到其他适配器

namespace ranges = std::ranges;
namespace views = std::views;

void print(auto result) {
    for (auto v : result) {
        fmt::print("{} ", v);
    }
    fmt::print("\n");
}

TEST(RangesViewsTest, ViewaAdaptor) {
    const std::vector<int> nums{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    EXPECT_TRUE(ranges::equal(ranges::take_view(nums, 5), std::vector{1, 2, 3, 4, 5}));
    EXPECT_TRUE(ranges::equal(nums | views::take(5), std::vector{1, 2, 3, 4, 5}));
    EXPECT_TRUE(ranges::equal(nums | views::take(5) | views::reverse, std::vector{5, 4, 3, 2, 1}));
    EXPECT_TRUE(
        ranges::equal(nums | views::filter([](int i) { return i % 2 == 0; }), std::vector{2, 4, 6, 8, 10}));
    EXPECT_TRUE(ranges::equal(nums | views::transform([](int i) { return i * i; }),
                              std::vector{1, 4, 9, 16, 25, 36, 49, 64, 81, 100}));

    // iota
    EXPECT_TRUE(ranges::equal(views::iota(1, 10), std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9}));
    {
        // 若省略 bound 参数, 则 iota 会生成无限序列
        auto rnums = views::iota(1) | views::take(200);
        EXPECT_EQ(rnums.front(), 1);
        EXPECT_EQ(*ranges::next(rnums.begin(), 200 - 1), 200);
        EXPECT_EQ(ranges::distance(rnums.begin(), rnums.end()), 200);
    }
}

TEST(RangesViewsTest, RangesAlgorithm) {
    {
        auto vec = std::vector{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
        ranges::sort(vec);
        EXPECT_TRUE(ranges::equal(vec, std::vector{1, 1, 2, 3, 3, 4, 5, 5, 6, 9}));
    }
    {
        auto vec = std::vector{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
        ranges::sort(views::drop(views::reverse(vec), 5)); // 丢弃后 5 个元素并逆序排序（从大到小）
        EXPECT_TRUE(ranges::equal(vec, std::vector{5, 4, 3, 1, 1, 9, 2, 6, 5, 3}));
    }
}

}  // namespace pyc
