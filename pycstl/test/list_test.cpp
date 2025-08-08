#include <list>

#include <gtest/gtest.h>

#include "pycstl/list.h"
#include "pycstl/test/utils.h"

static std::initializer_list<int> init_list{1, 2, 5, 7, 9, 392, 213, 1, -321, 12};

TEST(DISABLED_ListTest, Visit) {
    pycstl::List<int> arr{1, 2, 3, 4, 5};
    std::size_t i = 0;
    for (auto it = arr.cbegin(); it != arr.cend(); ++it) {
        const int& val = *it;
        fmt::println("arr[{}] = {}", i, val);
        i++;
    }

    i--;
    for (auto it = arr.crbegin(); it != arr.crend(); ++it) {
        const int& val = *it;
        fmt::println("arr[{}] = {}", i, val);
        i--;
    }
}

TEST(ListTest, Erase) {
    pycstl::List<int> arr{init_list};
    std::list<int> stl_arr{init_list};
    COMPARE_CONTENT(arr, stl_arr);

    ACTION_AND_COMPARE(
        arr, stl_arr, [](auto& container) { container.erase(container.cbegin()); }, COMPARE_CONTENT);

    ACTION_AND_COMPARE(
        arr, stl_arr,
        [](auto& container) {
            container.erase(std::next(container.cbegin(), 2), std::next(container.cbegin(), 4));
        },
        COMPARE_CONTENT);
}

TEST(ListTest, Remove) {
    pycstl::List<int> arr{init_list};
    std::list<int> stl_arr{init_list};
    COMPARE_CONTENT(arr, stl_arr);

    ACTION_AND_COMPARE(
        arr, stl_arr, [](auto& container) { container.remove(5); }, COMPARE_CONTENT);

    ACTION_AND_COMPARE(
        arr, stl_arr, [](auto& container) { container.remove(-99); }, COMPARE_CONTENT);

    auto remove_condition = [](int value) { return value > 100; };
    ACTION_AND_COMPARE(
        arr, stl_arr, [&](auto& container) { container.remove_if(remove_condition); }, COMPARE_CONTENT);
}
