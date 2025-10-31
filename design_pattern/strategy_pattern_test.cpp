#include <numeric>
#include <sstream>

#include <gtest/gtest.h>

#include "common/cin_redirect.h"
#include "design_pattern/api.h"
#include "design_pattern/inputer.h"

namespace pyc {

TEST(DesignPatternTest, StrategyPattern) {
    std::vector<int> v = {1, 2, 3, 4, 5};

    EXPECT_EQ(reduce(v, new SumReducer()), std::accumulate(v.begin(), v.end(), 0));
    EXPECT_EQ(reduce(v, new ProductReducer()), std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>()));
    EXPECT_EQ(reduce(v, new MinReducer()), *std::min_element(v.begin(), v.end()));
    EXPECT_EQ(reduce(v, new MaxReducer()), *std::max_element(v.begin(), v.end()));
}

// For test
struct CinInputerWithStop : Inputer {
    std::optional<int> fetch() override {
        int val;
        std::cin >> val;
        if (val == -1) {
            return std::nullopt;
        }
        return val;
    }
};

TEST(DesignPatternTest, MultiStrategyPattern) {
    std::vector<int> v = {1, 2, 3, 4, 5};

    constexpr std::string_view kInputString = "1\n2\n3\n4\n5\n-1\n";

    // 创建一个 string stream 对象用于模拟输入
    std::stringstream simulated_input;
    CinRedirect cin_redirect(simulated_input);

    simulated_input << kInputString;
    EXPECT_EQ(reduce(new CinInputerWithStop(), new SumReducer()), std::accumulate(v.begin(), v.end(), 0));
    simulated_input << kInputString;
    EXPECT_EQ(reduce(new CinInputerWithStop(), new ProductReducer()),
              std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>()));
    simulated_input << kInputString;
    EXPECT_EQ(reduce(new CinInputerWithStop(), new MinReducer()), *std::min_element(v.begin(), v.end()));
    simulated_input << kInputString;
    EXPECT_EQ(reduce(new CinInputerWithStop(), new MaxReducer()), *std::max_element(v.begin(), v.end()));

    EXPECT_EQ(reduce(new VectorInputer(v), new SumReducer()), std::accumulate(v.begin(), v.end(), 0));
    EXPECT_EQ(reduce(new VectorInputer(v), new ProductReducer()),
              std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>()));
    EXPECT_EQ(reduce(new VectorInputer(v), new MinReducer()), *std::min_element(v.begin(), v.end()));
    EXPECT_EQ(reduce(new VectorInputer(v), new MaxReducer()), *std::max_element(v.begin(), v.end()));
}

}  // namespace pyc
