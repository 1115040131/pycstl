#include <sstream>

#include <gtest/gtest.h>

#include "design_pattern/adapter.h"
#include "design_pattern/api.h"

namespace pyc {

TEST(DesignPatternTest, AdapterPattern) {
    constexpr std::string_view kInputString = "1\n2\n3\n4\n5\n-1\n";

    // 创建一个 string stream 对象用于模拟输入
    std::stringstream simulated_input;

    // 保存并重定向 cin 的缓冲区
    std::streambuf* origin_cin_buff = std::cin.rdbuf();
    std::cin.rdbuf(simulated_input.rdbuf());

    // 调用你的函数，它会从 cin 读取数据
    simulated_input << kInputString;
    EXPECT_EQ(reduce(new StopInputerAdapter(new CinInputer(), -1), new SumReducer()), 15);

    // 恢复原始的 cin 缓冲区
    std::cin.rdbuf(origin_cin_buff);

    std::vector<int> v = {1, 2, 3, -1, 4, 5};

    EXPECT_EQ(reduce(new VectorInputer(v), new SumReducer()), std::accumulate(v.begin(), v.end(), 0));
    EXPECT_EQ(reduce(new StopInputerAdapter(new VectorInputer(v), -1), new SumReducer()), 6);

    EXPECT_EQ(reduce(new FilterInputerAdapter(
                         new StopInputerAdapter(new VectorInputer(v), -1),
                         new FilterStrategyAnd(new FilterStrategyAbove(1), new FilterStrategyBelow(5))),
                     new SumReducer()),
              5);
    EXPECT_EQ(
        reduce(new FilterInputerAdapter(new VectorInputer(v), new FilterStrategyAnd(new FilterStrategyAbove(1),
                                                                                    new FilterStrategyBelow(5))),
               new ProductReducer()),
        24);
}

}  // namespace pyc
