#include <future>
#include <list>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>

#include "concurrency/thread_pool.h"

namespace pyc {
namespace concurrency {

/// @brief c++ 版本快速排序算法
template <typename T>
void QuickSortRecursive(std::vector<T>& arr, int start, int end) {
    if (start >= end) {
        return;
    }
    T key = arr[start];
    int left = start;
    int right = end;
    while (left < right) {
        while (arr[right] >= key && left < right) {
            right--;
        }
        while (arr[left] <= key && left < right) {
            left++;
        }
        std::swap(arr[left], arr[right]);
    }

    if (arr[left] < arr[start]) {
        std::swap(arr[left], arr[start]);
    }

    QuickSortRecursive(arr, start, left - 1);
    QuickSortRecursive(arr, right + 1, end);
}

template <typename T>
void QuickSortRecursive(std::vector<T>& arr, int len) {
    QuickSortRecursive(arr, 0, len - 1);
}

TEST(QuickSortTest, QuickSortRecursiveTest) {
    std::vector<int> arr = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    QuickSortRecursive(arr, arr.size());
    EXPECT_EQ(arr, std::vector<int>({1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9}));
}

/// @brief 函数式编程
template <typename T>
std::list<T> SequentialQuickSort(std::list<T> input) {
    if (input.empty()) {
        return input;
    }
    std::list<T> result;

    // 将 input 中第一个元素放入 result 中, 并从 input 中删除
    result.splice(result.begin(), input, input.begin());

    // 取 result 中的第一个元素, 作为分界点, 切割 input
    T const& pivot = *result.begin();

    // partition 将 input 分成两部分, 一部分是小于 pivot 的, 一部分是大于等于 pivot 的
    auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t) { return t < pivot; });

    // 将小于 pivot 的部分放入 lower_part
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    // 递归排序
    auto new_lower = SequentialQuickSort(std::move(lower_part));
    auto new_higher = SequentialQuickSort(std::move(input));

    // 将排序好的部分合并
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

TEST(QuickSortTest, SequentialQuickSortTest) {
    std::list<int> arr = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    auto result = SequentialQuickSort(arr);
    EXPECT_EQ(result, std::list<int>({1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9}));
}

/// @brief 并行快速排序
template <typename T>
std::list<T> ParallelQuickSort(std::list<T> input) {
    if (input.empty()) {
        return input;
    }
    std::list<T> result;

    // 将 input 中第一个元素放入 result 中, 并从 input 中删除
    result.splice(result.begin(), input, input.begin());

    // 取 result 中的第一个元素, 作为分界点, 切割 input
    T const& pivot = *result.begin();

    // partition 将 input 分成两部分, 一部分是小于 pivot 的, 一部分是大于等于 pivot 的
    auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t) { return t < pivot; });

    // 将小于 pivot 的部分放入 lower_part
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    // 递归排序
    std::future<std::list<T>> new_lower(std::async(&ParallelQuickSort<T>, std::move(lower_part)));
    auto new_higher = ParallelQuickSort(std::move(input));

    // 将排序好的部分合并
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}

TEST(QuickSortTest, ParallelQuickSortTest) {
    std::list<int> arr = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    auto result = ParallelQuickSort(arr);
    EXPECT_EQ(result, std::list<int>({1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9}));
}

/// @brief 线程池版本
template <typename T>
std::list<T> ThreadPoolQuickSort(std::list<T> input) {
    if (input.empty()) {
        return input;
    }
    std::list<T> result;

    // 将 input 中第一个元素放入 result 中, 并从 input 中删除
    result.splice(result.begin(), input, input.begin());

    // 取 result 中的第一个元素, 作为分界点, 切割 input
    T const& pivot = *result.begin();

    // partition 将 input 分成两部分, 一部分是小于 pivot 的, 一部分是大于等于 pivot 的
    auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t) { return t < pivot; });

    // 将小于 pivot 的部分放入 lower_part
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    // 递归排序
    auto new_lower = ThreadPool::GetInstance().Commit(ThreadPoolQuickSort<T>, std::move(lower_part));
    auto new_higher = ThreadPoolQuickSort(std::move(input));

    // 将排序好的部分合并
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}

TEST(QuickSortTest, ThreadPoolQuickSortTest) {
    std::list<int> arr = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    auto result = ThreadPoolQuickSort(arr);
    EXPECT_EQ(result, std::list<int>({1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9}));
}

}  // namespace concurrency
}  // namespace pyc
