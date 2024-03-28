#include <vector>

#include <gtest/gtest.h>

#include "concurrency/parallel_algorithm.h"

namespace pyc {
namespace concurrency {

TEST(ParallelAlgorithmTest, ParallelForEachTest) {
    std::vector<int> origin_data = {1, 2, 3, 4, 5};
    auto vec1 = origin_data;
    auto vec2 = origin_data;
    auto predicate = [](int& value) { value *= 2; };
    ParallelForEach(vec1.begin(), vec1.end(), predicate);
    std::for_each(vec2.begin(), vec2.end(), predicate);
    EXPECT_EQ(vec1, vec2);
}

TEST(ParallelAlgorithmTest, ParallelFindTest) {
    std::vector<int> origin_data = {1, 2, 3, 4, 5};
    {
        const int kTarget = 3;
        auto find1 = ParallelFind(origin_data.begin(), origin_data.end(), kTarget);
        auto find2 = std::find(origin_data.begin(), origin_data.end(), kTarget);
        EXPECT_EQ(find1, find2);
    }
    {
        const int kTarget = 6;
        auto find1 = ParallelFind(origin_data.begin(), origin_data.end(), kTarget);
        auto find2 = std::find(origin_data.begin(), origin_data.end(), kTarget);
        EXPECT_EQ(find1, find2);
    }
}

}  // namespace concurrency
}  // namespace pyc
