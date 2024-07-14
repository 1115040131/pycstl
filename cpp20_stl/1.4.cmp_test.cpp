#include <utility>

#include <gtest/gtest.h>

namespace pyc {

template <typename T, typename U>
constexpr bool cmp_less(T t, U u) noexcept {
    using UT = std::make_unsigned_t<T>;
    using UU = std::make_unsigned_t<U>;

    if constexpr (std::is_signed_v<T> == std::is_signed_v<U>) {
        return t < u;
    } else if constexpr (std::is_signed_v<T>) {
        return t < 0 || UT(t) < u;
    } else {
        return u >= 0 && t < UU(u);
    }
}

}  // namespace pyc

TEST(CmpTest, CmpTest) {
    int x{-3};
    unsigned y{7};

    // 无符号数和有符号数比较时, 有符号数会被转换为无符号数
    EXPECT_EQ(x < y, false);

    // C++20 标准在 <utility> 中包含了一组安全的整数比较函数
    EXPECT_EQ(std::cmp_equal(x, y), false);
    EXPECT_EQ(std::cmp_not_equal(x, y), true);
    EXPECT_EQ(std::cmp_less(x, y), true);
    EXPECT_EQ(std::cmp_less_equal(x, y), true);
    EXPECT_EQ(std::cmp_greater(x, y), false);
    EXPECT_EQ(std::cmp_greater_equal(x, y), false);

    EXPECT_EQ(pyc::cmp_less(x, y), true);
}