#include <vector>

#include <fmt/base.h>
#include <gtest/gtest.h>

namespace pyc {

auto is_div_by(int divisor) {
    return [divisor](int i) { return i % divisor == 0; };
}

TEST(LambdaTest, LambdaTest) {
    // 从C++20 开始, 可以在捕获括号之后指定模板参数. 这与模板参数相同
    auto p = []<typename T>(T v) { return v(); };
    // 现在可以在函数调用中传递一个匿名 lambda
    EXPECT_STREQ(p([] { return "lambda call lambda"; }), "lambda call lambda");

    // 可变说明符允许 lambda 修改它的捕获, lambda 默认限定为 const
    auto counter = [n = 0]() mutable { return ++n; };
    EXPECT_EQ(counter(), 1);
    EXPECT_EQ(counter(), 2);
}

// 可以使用一个简单的递归函数来级联 lambda
// 该函数返回一个匿名 lambda，该 lambda 再次调用该函数，直到耗尽参数包
template <typename T, typename... Ts>
auto concat(T t, Ts... ts) {
    if constexpr (sizeof...(ts) > 0) {
        return [&](auto... args) { return t(concat(ts...)(args...)); };
    } else {
        return t;
    }
}

TEST(LambdaTest, ConcatTest) {
    // 随着递归的展开，函数从右到左进行，从 plus() 开始
    auto combined = concat([](auto i) { return i * 2; }, [](auto i) { return i * 3; }, std::plus<int>{});
    EXPECT_EQ(combined(2, 3), 30);
}

// 将 lambda 包装在函数中，以创建用于算法谓词的自定义连接
template <typename F, typename A, typename B>
auto combine(F binary_func, A a, B b) {
    return [=](auto param) { return binary_func(a(param), b(param)); };
}

TEST(LambdaTest, CombineTest) {
    auto begin_with = [](std::string_view str) { return str.find("a") == 0; };
    auto end_with = [](std::string_view str) { return str.rfind("b") == str.size() - 1; };
    auto bool_and = [](const auto& lhs, const auto& rhs) { return lhs && rhs; };
    auto begin_and_end_with = combine(bool_and, begin_with, end_with);
    EXPECT_FALSE(begin_and_end_with(""));
    EXPECT_FALSE(begin_and_end_with("hello"));
    EXPECT_FALSE(begin_and_end_with("aabbaa"));
    EXPECT_TRUE(begin_and_end_with("adsfhb"));
}

}  // namespace pyc
