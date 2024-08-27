#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include <fmt/base.h>
#include <gtest/gtest.h>

namespace pyc {

#pragma region std::inner_product

TEST(OtherTest, InnerProductTest) {
    using std::numbers::pi;
    constexpr size_t kSize = 100;

    std::vector<double> ds(kSize);
    std::vector<int> is(kSize);

    for (size_t i = 0; i < kSize; i++) {
        ds[i] = 5.0 * std::sin(i * 2 * pi / 100);
        is[i] = static_cast<int>(std::round(ds[i]));
    }

    double error_suum = std::inner_product(ds.begin(), ds.end(), is.begin(), 0.0, std::plus<double>(),
                                           [](double a, double b) { return std::pow(a - b, 2); });
    EXPECT_NEAR(7.304, error_suum, 0.001);
}

#pragma endregion

#pragma region split

template <typename It, typename Oc, typename V, typename Pred>
It split(It it, It end_it, Oc& dest, const V& sep, Pred&& f) {
    using SliceContainer = typename Oc::value_type;
    while (it != end_it) {
        SliceContainer dest_elm{};
        auto slice{it};
        while (slice != end_it) {
            if (f(*slice, sep)) {
                break;
            }
            dest_elm.push_back(*slice++);
        }
        dest.push_back(dest_elm);
        if (slice != end_it) {
            it = slice + 1;
        } else {
            break;
        }
    }
    return it;
}

constexpr auto eq = [](const auto& el, const auto& sep) { return el == sep; };

// split() 的特化，默认使用 eq 操作符
template <typename It, typename Oc, typename V>
It split(It it, It end_it, Oc& dest, const V& sep) {
    return split(it, end_it, dest, sep, eq);
}

// 因为分割字符串对象是这个算法的常见用例，包含了一个用于特定目的的辅助函数:
template <typename Cin, typename Cout, typename V>
Cout& strsplit(const Cin& str, Cout& dest, const V& sep) {
    split(str.begin(), str.end(), dest, sep);
    return dest;
}

TEST(OtherTest, SplitTest) {
    constexpr char strsep{':'};
    const std::string str{"sync:x:4:65534:sync:/bin:/bin/sync"};
    {
        std::vector<std::string> dest_vs{};
        split(str.begin(), str.end(), dest_vs, strsep);
        EXPECT_EQ(dest_vs, (std::vector<std::string>{"sync", "x", "4", "65534", "sync", "/bin", "/bin/sync"}));
    }
    {
        std::vector<std::string> dest_vs{};
        strsplit(str, dest_vs, strsep);
        EXPECT_EQ(dest_vs, (std::vector<std::string>{"sync", "x", "4", "65534", "sync", "/bin", "/bin/sync"}));
    }

    constexpr int intsep{-1};
    std::vector<int> vi{1, 2, 3, 4, intsep, 5, 6, 7, 8, intsep, 9, 10, 11, 12};
    std::vector<std::vector<int>> dest_vi{};
    split(vi.begin(), vi.end(), dest_vi, intsep);
    EXPECT_EQ(dest_vi, (std::vector<std::vector<int>>{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}}));
}

#pragma endregion

#pragma region gather

template <typename It, typename Pred>
std::pair<It, It> gather(It first, It last, It pivot, Pred pred) {
    // std::stable_partition 使得满足谓词的元素排在不满足条件的元素之前，并保持它们原始的相对顺序。
    // 这与 std::partition 的不同之处在于后者不保证维护相对顺序
    return {std::stable_partition(first, pivot, std::not_fn(pred)), std::stable_partition(pivot, last, pred)};
}

TEST(OtherTest, GatherTest) {
    constexpr auto midit = [](auto& v) { return v.begin() + (v.end() - v.begin()) / 2; };
    constexpr auto is_even = [](int i) { return i % 2 == 0; };
    constexpr auto is_even_char = [](auto c) {
        if (c >= '0' && c <= '9') {
            return (c - '0') % 2 == 0;
        }
        return false;
    };

    {  // gather 到中间
        std::vector<int> vi{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto [first, last] = gather(vi.begin(), vi.end(), midit(vi), is_even);
        EXPECT_EQ(vi, (std::vector<int>{1, 3, 0, 2, 4, 6, 8, 5, 7, 9}));
        EXPECT_EQ(*first, 0);
        EXPECT_EQ(*(last - 1), 8);
    }
    {  // gather 到开头
        std::vector<int> vi{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto [first, last] = gather(vi.begin(), vi.end(), vi.begin(), is_even);
        EXPECT_EQ(vi, (std::vector<int>{0, 2, 4, 6, 8, 1, 3, 5, 7, 9}));
        EXPECT_EQ(*first, 0);
        EXPECT_EQ(*(last - 1), 8);
    }
    {  // gather 到结尾
        std::vector<int> vi{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto [first, last] = gather(vi.begin(), vi.end(), vi.end(), is_even);
        EXPECT_EQ(vi, (std::vector<int>{1, 3, 5, 7, 9, 0, 2, 4, 6, 8}));
        EXPECT_EQ(*first, 0);
        EXPECT_EQ(*(last - 1), 8);
    }
    {
        std::string jenny{"867-5309"};
        auto [first, last] = gather(jenny.begin(), jenny.end(), jenny.end(), is_even_char);
        EXPECT_EQ(jenny, "7-539860");
        EXPECT_EQ(*first, '8');
        EXPECT_EQ(*(last - 1), '0');
    }
}

#pragma endregion

#pragma region delws

template <typename T>
constexpr bool isws(const T& c) {
    constexpr const T whitespace[]{" \t\r\n\v\f"};
    for (const T& wsc : whitespace) {
        if (c == wsc) {
            return true;
        }
    }
    return false;
}

std::string delws(std::string_view s) {
    std::string outstr{s};
    auto iter = std::unique(outstr.begin(), outstr.end(), [](char a, char b) { return isws(a) && isws(b); });
    outstr.erase(iter, outstr.end());
    outstr.shrink_to_fit();
    return outstr;
}

TEST(OtherTest, DelwsTest) {
    EXPECT_EQ(delws("  \t  \n  \r  \v  \f  "), " ");
    EXPECT_EQ(delws("  \t  \n  \r  \v  \f  a  b  c  "), " a b c ");
    EXPECT_EQ(delws("big bad \t wolf"), "big bad wolf");
}

#pragma endregion

}  // namespace pyc
