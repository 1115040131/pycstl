#include <array>

#include <gtest/gtest.h>

#include "pycstl/array.h"

using namespace pycstl;

void modify_array(Array<int, 32>& arr) {
    for (int i = 0; i <= 32; i++) {
        arr.at(i) = 12;
    }
}

TEST(ArrayTest, ModifyArray) {
    Array<int, 32> arr{1, 2, 3};
    EXPECT_THROW(modify_array(arr), std::out_of_range);

    arr.fill(17);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(arr[i], 17);
    }
}

TEST(ArrayTest, SizeCompare) {
    Array<int, 32> arr{1, 2, 3};
    EXPECT_EQ(decltype(arr)::Size(), 32);
    EXPECT_EQ(arr.size(), 32);

    Array<int8_t, decltype(arr)::Size()> arr2;
    EXPECT_EQ(arr2.size(), 32);

    Array arr3{4, 5, 6};
    EXPECT_EQ(arr3.size(), 3);

    int ca[] = {7, 8, 9, 10};
    auto arr4 = to_array(ca);
    EXPECT_EQ(arr4.size(), 4);
}

TEST(ArrayTest, IteratorVisit) {
    Array<int, 32> arr{1, 2, 3};
    for (auto it = arr.begin(); it != arr.end(); it++) {
        *it = 13;
    }
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(arr[i], 13);
    }

    int count = 0;
    for (auto& ele : arr) {
        ele = count++;
    }
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(arr[i], i);
    }
}
