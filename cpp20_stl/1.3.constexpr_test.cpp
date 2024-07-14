#include <numeric>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// C++20 开始 std::string 和 std::vector 具有限定的构造和析构函数, 可以在编译期使用, 但分配的内存必须也在编译时释放
constexpr auto use_string() {
    std::string str{"string"};
    return str.size();
}

constexpr auto use_vector() {
    std::vector<int> vec{1, 2, 3, 4, 5};
    return std::accumulate(vec.begin(), vec.end(), 0);
}

constexpr auto use_vector2() {
    std::vector<int> vec{1, 2, 3, 4, 5};
    return vec;
}

TEST(ContexprTest, ContexprTest) {
    EXPECT_EQ(use_string(), 6);
    EXPECT_EQ(use_vector(), 15);

    // 但是若试图在运行时环境中使用结果, 会得到一个在常量求值期间分配内存的错误
    // constexpr auto vec = use_vector2();
}

//constexpr: https://zh.cppreference.com/w/cpp/language/constexpr