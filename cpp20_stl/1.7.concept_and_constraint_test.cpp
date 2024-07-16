#include <concepts>

#include <gtest/gtest.h>

namespace pyc {

// 可以使用 <type_traits> 头文件中预定义的特性, 或者自定义的特性, 就像模板变量一样.
// 为了在约束中使用, 该变量必须返回 constexpr bool
template <typename T>
constexpr bool is_gt_byte{sizeof(T) > 1};

// && 是一个连接
// || 是一个析取
template <typename T>
concept Numeric = is_gt_byte<T> && (std::integral<T> || std::floating_point<T>);

// 约束函数模板
template <typename T>
    requires Numeric<T>
T arg42(const T& arg) {
    return arg + 42;
}

template <typename T>
std::string arg42(const T& arg) {
    return "not Numeric<T>";
}

// 约束类模板
template <typename T>
    requires Numeric<T>
struct Num {
    T n;
    Num(T n) : n(n) {}
};

// 约束变量模板
template <typename T>
    requires std::floating_point<T>
T pi{3.1415926535897932385};

// 使用概念或约束有几种不同的方法
namespace v1 {
// 用 require 关键字定义一个概念或约束
template <typename T>
    requires Numeric<T>
T arg42(const T& arg) {
    return arg + 42;
}
}  // namespace v1

namespace v2 {
// 在模板声明中使用概念
template <Numeric T>
T arg42(const T& arg) {
    return arg + 42;
}
}  // namespace v2

namespace v3 {
// 在函数签名中使用 require 关键字
template <typename T>
T arg42(const T& arg)
    requires Numeric<T>
{
    return arg + 42;
}
}  // namespace v3

namespace v4 {
// 在参数列表中使用概念来简化函数模板
auto arg42(const Numeric auto& arg) { return arg + 42; }
}  // namespace v4

TEST(ConceptAndConstraintTest, ConceptAndConstraintTest) {
    EXPECT_EQ(arg42(7), 49);
    EXPECT_EQ(arg42(7.0), 49.0);
    EXPECT_EQ(arg42(7.0f), 49.0f);
    EXPECT_EQ(arg42(7.0L), 49.0L);
    EXPECT_EQ(arg42("7"), "not Numeric<T>");

    EXPECT_EQ(v1::arg42(7), 49);
    EXPECT_EQ(v2::arg42(7), 49);
    EXPECT_EQ(v3::arg42(7), 49);
    EXPECT_EQ(v4::arg42(7), 49);
}

}  // namespace pyc
