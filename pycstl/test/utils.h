#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>

#define ACTION_AND_COMPARE(container1, container2, ACTION, COMPARE) \
    ACTION(container1);                                             \
    ACTION(container2);                                             \
    COMPARE(container1, container2);

/// @brief 比较容器元素, 大小, 容量
#define COMPARE_ALL(container1, container2)                        \
    {                                                              \
        EXPECT_EQ(container1.size(), container2.size());           \
        EXPECT_EQ(container1.capacity(), container2.capacity());   \
        auto iter1 = container1.begin();                           \
        auto iter2 = container2.begin();                           \
        for (size_t i = 0; i < container2.size(); i++) {           \
            EXPECT_EQ(*iter1, *iter2) << fmt::format("pos={}", i); \
            iter1++;                                               \
            iter2++;                                               \
        }                                                          \
    }

/// @brief 比较容器元素, 大小
#define COMPARE_CONTENT(container1, container2)                    \
    {                                                              \
        EXPECT_EQ(container1.size(), container2.size());           \
        auto iter1 = container1.begin();                           \
        auto iter2 = container2.begin();                           \
        for (size_t i = 0; i < container2.size(); i++) {           \
            EXPECT_EQ(*iter1, *iter2) << fmt::format("pos={}", i); \
            iter1++;                                               \
            iter2++;                                               \
        }                                                          \
    }
