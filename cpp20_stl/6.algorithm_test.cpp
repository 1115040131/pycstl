#include <algorithm>
#include <array>
#include <list>
#include <numbers>
#include <random>
#include <ranges>
#include <string>
#include <vector>

#include <fmt/base.h>
#include <gtest/gtest.h>

#include "utils.h"

namespace pyc {

#pragma region std::copy std::move

template <typename InputIt, typename OutputIt>
OutputIt copy(InputIt first, InputIt last, OutputIt result) {
    while (first != last) {
        *result++ = *first++;
    }
    return result;
}

TEST(AlgorithmTest, CopyTest) {
    std::vector v1{1, 2, 3, 4, 5};
    {
        // copy() 算法不为目标分配空间。因此，v2 必须已经有用于复制的空间
        decltype(v1) v2(v1.size());
        std::copy(v1.begin(), v1.end(), v2.begin());
        EXPECT_EQ(v1, v2);
    }
    {
        // 或者，使用 back_inserter()
        decltype(v1) v2;
        std::copy(v1.begin(), v1.end(), std::back_inserter(v2));
        EXPECT_EQ(v1, v2);
    }
    {
        decltype(v1) v2;
        std::ranges::copy(v1, std::back_inserter(v2));
        EXPECT_EQ(v1, v2);
    }
    {
        decltype(v1) v2;
        std::ranges::copy_n(v1.begin(), 3, std::back_inserter(v2));
        EXPECT_EQ(v2, (std::vector{1, 2, 3}));
    }
    {
        decltype(v1) v2;
        std::ranges::copy_if(v1, std::back_inserter(v2), [](int i) { return i % 2 == 1; });
        EXPECT_EQ(v2, (std::vector{1, 3, 5}));
    }
    {
        decltype(v1) v2(v1.size());
        pyc::copy(v1.begin(), v1.end(), v2.begin());
        EXPECT_EQ(v1, v2);
    }
}

TEST(AlgorithmTest, MoveTest) {
    std::vector<std::string> v1{"alpha", "beta", "gamma", "delta", "epsilon"};
    auto tmp = v1;
    decltype(v1) v2;
    std::ranges::move(v1, std::back_inserter(v2));
    EXPECT_EQ(v1, (std::vector<std::string>{"", "", "", "", ""}));
    EXPECT_EQ(v2, tmp);
}

#pragma endregion

#pragma region join

template <typename InputIt>
std::ostream& join(InputIt it, InputIt end_it, std::ostream& os, std::string_view sep = "") {
    if (it != end_it) {
        os << *it++;
        while (it != end_it) {
            os << sep << *it++;
        }
    }
    return os;
}

template <typename InputIt>
std::string join(InputIt it, InputIt end_it, std::string_view sep = "") {
    std::ostringstream os;
    join(it, end_it, os, sep);
    return os.str();
}

std::string join(const auto& container, std::string_view sep = "") {
    return join(container.begin(), container.end(), sep);
}

TEST(AlgorithmTest, JoinTest) {
    std::vector<std::string> v{"alpha", "beta", "gamma", "delta", "epsilon"};

    // ranges::views 库有一个 join() 函数，但不提供分隔符
    std::ostringstream os;
    for (auto c : std::views::join(v)) {
        os << c;
    }
    EXPECT_EQ(os.str(), "alphabetagammadeltaepsilon");

    EXPECT_EQ(join(v.begin(), v.end(), ", "), "alpha, beta, gamma, delta, epsilon");
    EXPECT_EQ(join(v, ", "), "alpha, beta, gamma, delta, epsilon");

    EXPECT_EQ(join(std::list{std::numbers::pi, std::numbers::e, std::numbers::sqrt2}, ", "),
              "3.14159, 2.71828, 1.41421");
}

#pragma endregion

#pragma region std::sort

void randomize(auto& container) {
    static std::random_device rd;
    static std::default_random_engine gen(rd());
    std::shuffle(container.begin(), container.end(), gen);
}

TEST(AlgorithmTest, SortTest) {
    std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    randomize(v);
    printc(v);

    std::ranges::sort(v);
    EXPECT_EQ(v, (std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));

    // partial_sort() 对容器进行排序，使中间之前的元素排序。中间之后的元素不保证是原来的顺序
    randomize(v);
    std::ranges::partial_sort(v, v.begin() + v.size() / 2);
    EXPECT_TRUE(std::ranges::equal(v | std::views::take(v.size() / 2), std::vector{1, 2, 3, 4, 5}));

    // partition() 算法不进行任何排序，其会重新排列容器，使某些元素出现在容器的前面
    randomize(v);
    std::ranges::partition(v, [](int i) { return i > 5; });
    for (int i = 0; i < 5; i++) {
        EXPECT_TRUE(v[i] > 5);
    }
    printc(v);
}

#pragma endregion

#pragma region std::transform

TEST(AlgorithmTest, TransformTest) {
    std::vector v1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    decltype(v1) v2;
    std::transform(v1.begin(), v1.end(), std::back_inserter(v2), [](int i) { return i * i; });
    EXPECT_EQ(v2, (std::vector{1, 4, 9, 16, 25, 36, 49, 64, 81, 100}));

    EXPECT_TRUE(std::ranges::equal(v1 | std::views::transform([](int i) { return i * i; }),
                                   std::vector{1, 4, 9, 16, 25, 36, 49, 64, 81, 100}));
}

#pragma endregion

#pragma region std::find

TEST(AlgorithmTest, FindTest) {
    struct City {
        std::string name;
        int population;

        bool operator==(const City& that) const { return name == that.name; }
    };

    std::vector<City> cities{
        {"London", 9425622},
        {"Berlin", 3566791},
        {"Tokyo", 37435191},
        {"Cairo", 20485965},
    };
    EXPECT_TRUE(std::ranges::find(cities, City{"Berlin"}) != cities.end());
    EXPECT_FALSE(std::ranges::find(cities, City{"23"}) != cities.end());

    auto iter = std::ranges::find_if(cities, [](const City& city) { return city.population > 10'000'000; });
    EXPECT_TRUE(iter != cities.end() && iter->name == "Tokyo");

    EXPECT_TRUE(std::ranges::equal(
        cities | std::views::filter([](const City& city) { return city.population > 10'000'000; }),
        std::vector{City{"Tokyo", 37435191}, City{"Cairo", 20485965}}));
}

#pragma endregion

#pragma region std::clamp

template <typename T>
constexpr const T& clamp(const T& value, const T& low, const T& high) {
    return (value < low) ? low : (high < value) ? high : value;
}

TEST(AlgorithmTest, ClampTest) {
    std::vector prototype = {0, -12, 2001, 4, 5, -14, 100, 200, 30000};
    constexpr int kLow{0};
    constexpr int kHigh{500};

    {
        decltype(prototype) v;
        std::ranges::transform(prototype, std::back_inserter(v),
                               [kLow, kHigh](int i) { return std::clamp(i, kLow, kHigh); });
        EXPECT_EQ(v, (std::vector{0, 0, 500, 4, 5, 0, 100, 200, 500}));
    }
    {
        decltype(prototype) v;
        std::ranges::transform(prototype, std::back_inserter(v),
                               [kLow, kHigh](int i) { return clamp(i, kLow, kHigh); });
        EXPECT_EQ(v, (std::vector{0, 0, 500, 4, 5, 0, 100, 200, 500}));
    }
}

#pragma endregion

#pragma region std::sample

int iround(double x) { return static_cast<int>(std::round(x)); }

TEST(AlgorithmTest, SampleTest) {
    constexpr size_t kData{10000};
    constexpr size_t kSample{500};
    constexpr int kMean{0};
    constexpr int kStdDev{3};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(kMean, kStdDev);

    // gen 对象是硬件随机数生成器，其可传递给 normal_distribution 对象的 dist()，再取整
    std::array<int, kData> data;
    std::ranges::generate(data, [&dist, &gen] { return iround(dist(gen)); });

    // 使用 sample() 算法用随机数据点填充数组
    std::array<int, kSample> samples;
    std::ranges::sample(data, samples.begin(), kSample, gen);

    // 创建一个直方图来分析样本
    std::map<int, size_t> hist;
    for (int i : samples) {
        ++hist[i];
    }

    fmt::println("{:>3} {:>5} {:<}/3", "n", "count", "graph");
    for (const auto& [value, count] : hist) {
        fmt::println("{:>3} ({:>3}) {:<}", value, count, std::string(count / 3, '*'));
    }
}

#pragma endregion

#pragma region std::next_permutation

TEST(AlgorithmTest, NextPermutationTest) {
    {
        std::vector<std::string> vs{"dog", "cat", "velociraptor"};
        std::ranges::sort(vs);
        EXPECT_EQ(vs, (std::vector<std::string>{"cat", "dog", "velociraptor"}));

        std::vector<std::vector<std::string>> result;
        do {
            result.push_back(vs);
        } while (std::ranges::next_permutation(vs).found);

        EXPECT_EQ(result, (std::vector<std::vector<std::string>>{
                              {"cat", "dog", "velociraptor"},
                              {"cat", "velociraptor", "dog"},
                              {"dog", "cat", "velociraptor"},
                              {"dog", "velociraptor", "cat"},
                              {"velociraptor", "cat", "dog"},
                              {"velociraptor", "dog", "cat"},
                          }));
    }

    {
        std::vector<int> vi{1, 1, 3};
        std::vector<std::vector<int>> result;
        do {
            result.push_back(vi);
        } while (std::ranges::next_permutation(vi).found);
        EXPECT_EQ(result, (std::vector<std::vector<int>>{
                              {1, 1, 3},
                              {1, 3, 1},
                              {3, 1, 1},
                          }));
    }
}

#pragma endregion

#pragma region std::merge

TEST(AlgorithmTest, MergeTest) {
    std::vector<std::string> vs1{"dog", "cat", "velociraptor"};
    std::vector<std::string> vs2{"kirk", "sulu", "spock"};
    std::ranges::sort(vs1);
    EXPECT_EQ(vs1, (std::vector<std::string>{"cat", "dog", "velociraptor"}));
    std::ranges::sort(vs2);
    EXPECT_EQ(vs2, (std::vector<std::string>{"kirk", "spock", "sulu"}));

    std::vector<std::string> result;
    std::ranges::merge(vs1, vs2, std::back_inserter(result));
    EXPECT_EQ(result, (std::vector<std::string>{"cat", "dog", "kirk", "spock", "sulu", "velociraptor"}));
}

#pragma endregion

}  // namespace pyc
