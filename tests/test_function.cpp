#include <format>
#include <iostream>

#include <gtest/gtest.h>

#include "function.h"

void repeatTwice(pycstl::Function<void(int)> const& func) {
    func(1);
    func(2);
}

void func_hello(int i) {
    std::cout << std::format("#{} Hello\n", i);
}

struct FuncPrintnum {
    void operator()(int i) const {
        std::cout << std::format("#{} Numbers are: {}, {}\n", i, x, y);
    }
    int x;
    int y;
};

/// @brief function 接收函数
TEST(test_function, test_func) {
    testing::internal::CaptureStdout();
    repeatTwice(func_hello);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "#1 Hello\n#2 Hello\n");
}

/// @brief function 接收结构体 (lambda 展开)
TEST(test_function, test_struct) {
    testing::internal::CaptureStdout();
    FuncPrintnum func_printnum{2, 4};
    repeatTwice(func_printnum);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "#1 Numbers are: 2, 4\n#2 Numbers are: 2, 4\n");
}

/// @brief function 接收 lambda
TEST(test_function, test_lambda) {
    testing::internal::CaptureStdout();
    int x = 3;
    int y = 9;
    repeatTwice([=](int i) {
        std::cout << std::format("#{} Numbers are: {}, {}\n", i, x, y);
    });
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "#1 Numbers are: 3, 9\n#2 Numbers are: 3, 9\n");
}
