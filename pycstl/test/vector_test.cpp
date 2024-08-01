
#include <vector>

#include <fmt/base.h>
#include <gtest/gtest.h>

#include "pycstl/test/utils.h"
#include "pycstl/vector.h"

TEST(VectorTest, test_construction) {
    pycstl::Vector<int> vec1;
    std::vector<int> stl_vec1;
    EXPECT_EQ(sizeof(vec1), sizeof(stl_vec1));
    COMPARE_ALL(vec1, stl_vec1);

    pycstl::Vector<int> vec2(0);
    std::vector<int> stl_vec2(0);
    COMPARE_ALL(vec2, stl_vec2);

    pycstl::Vector<int> vec3(7);
    std::vector<int> stl_vec3(7);
    COMPARE_ALL(vec3, stl_vec3);

    pycstl::Vector<int> vec4(7, 8);
    std::vector<int> stl_vec4(7, 8);
    COMPARE_ALL(vec4, stl_vec4);

    for (std::size_t i = 0; i < stl_vec4.size(); i++) {
        vec4[i] = static_cast<int>(100 + i);
        stl_vec4[i] = static_cast<int>(100 + i);
    }
    COMPARE_ALL(vec4, stl_vec4);

    pycstl::Vector<int> vec5(vec4.begin(), vec4.end());
    std::vector<int> stl_vec5(stl_vec4.begin(), stl_vec4.end());
    COMPARE_ALL(vec5, stl_vec5);

    auto vec6 = vec4;
    auto stl_vec6 = stl_vec4;
    COMPARE_ALL(vec6, stl_vec6);

    vec1 = vec4;
    stl_vec1 = stl_vec4;
    COMPARE_ALL(vec1, stl_vec1);

    vec1.assign(3, 9);
    stl_vec1.assign(3, 9);
    COMPARE_ALL(vec1, stl_vec1);

    vec1.assign(50, 91);
    stl_vec1.assign(50, 91);
    COMPARE_ALL(vec1, stl_vec1);

    vec1.assign(vec4.begin(), vec4.end());
    stl_vec1.assign(stl_vec4.begin(), stl_vec4.end());
    COMPARE_ALL(vec1, stl_vec1);

    auto bar = std::move(vec1);
    COMPARE_CONTENT(bar, stl_vec1);
    EXPECT_EQ(vec1.size(), 0);
}

TEST(VectorTest, test_initialization) {
    pycstl::Vector<int> vec1{17, 24, 145, -12, 31};
    std::vector<int> vec2{17, 24, 145, -12, 31};
    COMPARE_ALL(vec1, vec2);

    vec1 = {102, 123, 65, 238, 209, -3824, 4673};
    vec2 = {102, 123, 65, 238, 209, -3824, 4673};
    COMPARE_ALL(vec1, vec2);

    vec1.assign({17, 24, 145, -12, 31});
    vec2.assign({17, 24, 145, -12, 31});
    COMPARE_ALL(vec1, vec2);
}

TEST(VectorTest, test_modify) {
    pycstl::Vector<int> vec1{1, 2, 3};
    std::vector<int> vec2{1, 2, 3};
    COMPARE_ALL(vec1, vec2);

    vec1.resize(6);
    vec2.resize(6);
    COMPARE_ALL(vec1, vec2);

    auto bar1 = vec1;
    auto bar2 = vec2;
    COMPARE_ALL(bar1, bar2);

    vec1.resize(2);
    vec2.resize(2);
    COMPARE_ALL(vec1, vec2);
    COMPARE_ALL(bar1, bar2);
}

TEST(VectorTest, test_erase_insert) {
    pycstl::Vector<int> vec1;
    std::vector<int> vec2;
    vec1.reserve(16);
    vec2.reserve(16);
    for (std::size_t i = 0; i < vec2.capacity(); i++) {
        vec1.push_back(static_cast<int>(i));
        vec2.push_back(static_cast<int>(i));
    }
    COMPARE_ALL(vec1, vec2);

    vec1.erase(vec1.begin() + 4);
    vec2.erase(vec2.begin() + 4);
    COMPARE_ALL(vec1, vec2);

    vec1.erase(vec1.begin() + 4, vec1.begin() + 8);
    vec2.erase(vec2.begin() + 4, vec2.begin() + 8);
    COMPARE_ALL(vec1, vec2);

    vec1.insert(vec1.begin(), 10, 100);
    vec2.insert(vec2.begin(), 10, 100);
    COMPARE_ALL(vec1, vec2);

    vec1.insert(vec1.end(), 10, 200);
    vec2.insert(vec2.end(), 10, 200);
    COMPARE_ALL(vec1, vec2);

    vec1.insert(vec1.begin() + 6, 10, 200);
    vec2.insert(vec2.begin() + 6, 10, 200);
    COMPARE_ALL(vec1, vec2);

    vec1.insert(vec1.begin() + 9, {3, 7, 12, -1});
    vec2.insert(vec2.begin() + 9, {3, 7, 12, -1});
    COMPARE_ALL(vec1, vec2);

    vec1.erase(vec1.begin());
    vec2.erase(vec2.begin());
    COMPARE_ALL(vec1, vec2);

    vec1.erase(vec1.end() - 1);
    vec2.erase(vec2.end() - 1);
    COMPARE_ALL(vec1, vec2);

    vec1.erase(vec1.end() - 3, vec1.end());
    vec2.erase(vec2.end() - 3, vec2.end());
    COMPARE_ALL(vec1, vec2);
}

TEST(VectorTest, test_emplace) {
    struct Test {
        int x;
        int y;

        bool operator==(const Test& that) const { return x == that.x && y == that.y; }
    };

    pycstl::Vector<Test> vec1;
    vec1.emplace_back(2, 4);
    vec1.emplace_back() = {1, 3};
    vec1.emplace_back().x = 6;

    std::vector<Test> vec2;
    vec2.emplace_back(2, 4);
    vec2.emplace_back() = {1, 3};
    vec2.emplace_back().x = 6;
    COMPARE_ALL(vec1, vec2);
}