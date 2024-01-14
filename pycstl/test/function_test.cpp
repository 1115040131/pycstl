#include <fmt/printf.h>
#include <gtest/gtest.h>

#include "pycstl/function.h"

void repeatTwice(pycstl::Function<void(int)> const& func) {
    func(1);
    func(2);
}

void func_hello(int i) { fmt::println("#{} Hello", i); }

struct FuncPrintnum {
    void operator()(int i) const { fmt::println("#{} Numbers are: {}, {}", i, x, y); }
    int x;
    int y;
};

/// @brief function 接收函数
TEST(FunctionTest, SetFunction) {
    testing::internal::CaptureStdout();
    repeatTwice(func_hello);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "#1 Hello\n#2 Hello\n");
}

/// @brief function 接收结构体 (lambda 展开)
TEST(FunctionTest, SetStruct) {
    testing::internal::CaptureStdout();
    FuncPrintnum func_printnum{2, 4};
    repeatTwice(func_printnum);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "#1 Numbers are: 2, 4\n#2 Numbers are: 2, 4\n");
}

/// @brief function 接收 lambda
TEST(FunctionTest, SetLambda) {
    testing::internal::CaptureStdout();
    int x = 3;
    int y = 9;
    repeatTwice([=](int i) { fmt::println("#{} Numbers are: {}, {}", i, x, y); });
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "#1 Numbers are: 3, 9\n#2 Numbers are: 3, 9\n");
}
