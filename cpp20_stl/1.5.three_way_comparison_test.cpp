#include <compare>

#include <gtest/gtest.h>

namespace pyc {

TEST(ThreeWayComparisonTest, ThreeWayComparisonTest) {
    {
        const int a = 7;
        const int b = 42;
        EXPECT_TRUE((a <=> b) < 0);
    }

    // 若操作数为整型，则返回值为 std::strong_ordering
    // 若操作数为浮点型，则返回值为 std::partial_ordering
}

struct Num {
    int a;
    constexpr Num(int a) : a(a) {}

    // 默认的 <=> 已经是 constexpr 了
    auto operator<=>(const Num&) const = default;
};

TEST(ThreeWayComparisonTest, NumTest) {
    constexpr Num a{7};
    constexpr Num b{7};
    constexpr Num c{42};

    EXPECT_TRUE(a < c);
    EXPECT_TRUE(c > a);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= c);
    EXPECT_TRUE(c >= a);
    EXPECT_TRUE(c != a);
}

struct Frac {
    long n;
    long d;
    constexpr Frac(int a, int b) : n{a}, d{b} {}
    constexpr double db1() const { return static_cast<double>(n) / static_cast<double>(d); }
    constexpr auto operator<=>(const Frac& rhs) const { return db1() <=> rhs.db1(); }
    // 表达式重写规则不会使用自定义操作符 <=> 重载重写 == 和 != 操作符
    constexpr bool operator==(const Frac& rhs) const { return db1() <=> rhs.db1() == 0; }
};

TEST(ThreeWayComparisonTest, FracTest) {
    constexpr Frac a{10, 15};
    constexpr Frac b{2, 3};
    constexpr Frac c{5, 3};

    EXPECT_TRUE(a < c);
    EXPECT_TRUE(c > a);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= c);
    EXPECT_TRUE(c >= a);
    EXPECT_TRUE(c != a);
}

}  // namespace pyc
