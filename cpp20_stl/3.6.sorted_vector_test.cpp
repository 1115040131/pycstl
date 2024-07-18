#include <algorithm>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <gtest/gtest.h>

namespace pyc {

// 在插入的同时保证 vector 顺序
void insert_sorted(std::vector<std::string>& vec, const std::string& value) {
    auto iter = std::ranges::lower_bound(vec, value);
    vec.insert(iter, value);
}

TEST(SortedVectorTest, SortedVectorTest) {
    std::vector<std::string> vec{"Miles", "Hendrix", "Beatles", "Zappa", "Shostakovich"};
    EXPECT_FALSE(std::ranges::is_sorted(vec));
    std::ranges::sort(vec);
    EXPECT_TRUE(std::ranges::is_sorted(vec));

    insert_sorted(vec, "Ella");
    EXPECT_TRUE(std::ranges::is_sorted(vec));
    insert_sorted(vec, "Stones");
    EXPECT_TRUE(std::ranges::is_sorted(vec));
}

}  // namespace pyc
