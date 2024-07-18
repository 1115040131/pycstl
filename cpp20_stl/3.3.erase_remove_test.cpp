#include <algorithm>
#include <list>
#include <map>
#include <ranges>
#include <vector>

#include <fmt/core.h>
#include <gtest/gtest.h>

namespace pyc {

template <typename Tc, typename Tv>
void remove_value(Tc& container, const Tv& value) {
    container.erase(std::remove(container.begin(), container.end(), value), container.end());
}

TEST(EraseRemoveTest, EraseVector) {
    // 两步过程
    {
        std::vector v{1, 2, 3, 4, 5};
        auto iter = std::remove(v.begin(), v.end(), 3);
        EXPECT_EQ(v, (std::vector{1, 2, 4, 5, 5}));
        v.erase(iter, v.end());
        EXPECT_EQ(v, (std::vector{1, 2, 4, 5}));
    }
    {
        std::vector v{1, 2, 3, 4, 5};
        remove_value(v, 3);
        EXPECT_EQ(v, (std::vector{1, 2, 4, 5}));
    }
    // C++20 提供了 std::erase 和 std::erase_if
    {
        std::vector v{1, 2, 3, 4, 5};
        // std::erase 函数的 vector 版本定义在 <vector> 头文件中
        std::erase(v, 3);  // 和 remove_value 一样
        EXPECT_EQ(v, (std::vector{1, 2, 4, 5}));
    }
    {
        std::vector v{1, 2, 3, 4, 5};
        std::erase_if(v, [](int i) { return i % 2 == 0; });
        EXPECT_EQ(v, (std::vector{1, 3, 5}));
    }
}

TEST(EraseRemoveTest, EraseList) {
    std::list l{1, 2, 3, 4, 5};
    // std::erase 函数的 list 版本定义在 <list> 头文件中
    std::erase(l, 3);
    EXPECT_EQ(l, (std::list{1, 2, 4, 5}));
}

TEST(EraseRemoveTest, EraseMap) {
    std::map<int, std::string> m{{1, "uno"}, {2, "dos"}, {3, "tres"}, {4, "quatro"}, {5, "cinco"}};
    std::erase_if(m, [](const auto& iter) { return iter.first % 2 == 0; });
    EXPECT_EQ(m, (std::map<int, std::string>{{1, "uno"}, {3, "tres"}, {5, "cinco"}}));
}

// 如果不在意元素顺序, 可以在常数时间内删除元素
template <typename T>
void quick_delete(T& container, size_t idx) {
    if (idx < container.size()) {
        container[idx] = std::move(container.back());
        container.pop_back();
    }
}

template <typename T>
void quick_delete(T& container, typename T::iterator iter) {
    if (iter < container.end()) {
        *iter = std::move(container.back());
        container.pop_back();
    }
}

TEST(EraseRemoveTest, QuickDelete) {
    std::vector vec{12, 196, 47, 38, 19};
    quick_delete(vec, std::ranges::find(vec, 47));
    EXPECT_EQ(vec, (std::vector{12, 196, 19, 38}));
    quick_delete(vec, 1);
    EXPECT_EQ(vec, (std::vector{12, 38, 19}));
}

}  // namespace pyc
