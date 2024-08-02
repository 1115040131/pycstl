#include <fstream>
#include <regex>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <gtest/gtest.h>

namespace pyc {

#pragma region 删除字符串中的空白

std::string trimstr(std::string_view s) {
    constexpr std::string_view whitespace{" \t\r\n\v\f"};
    if (s.empty()) {
        return {};
    }

    auto start = s.find_first_not_of(whitespace);
    if (start == std::string_view::npos) {
        return {};
    }
    auto end = s.find_last_not_of(whitespace);
    return std::string{s.substr(start, end - start + 1)};
}

TEST(StringTest, DeleteSpaceTest) {
    std::string s{" \t ten-thumbed input \t \n \t "};
    EXPECT_EQ(trimstr(s), "ten-thumbed input");
}

#pragma endregion

#pragma region 统计文件中的单词数

size_t wordcount(auto& is) {
    using it_t = std::istream_iterator<std::string>;
    return std::distance(it_t{is}, it_t{});
}

TEST(StringTest, WordCountTest) {
    std::ifstream input_file("cpp20_stl/resource/the-raven.txt");
    EXPECT_TRUE(input_file.is_open());
    EXPECT_EQ(wordcount(input_file), 1068);
}

#pragma endregion

#pragma region 从文件初始化结构体

struct CityInfo {
    std::string name;
    unsigned long population;
    double latitude;
    double longitude;

    bool operator==(const CityInfo&) const = default;
};

std::istream& operator>>(std::istream& is, CityInfo& city) {
    is >> std::ws;  // 丢弃输入流中前面的空格
    std::getline(is, city.name);
    is >> city.population >> city.latitude >> city.longitude;
    return is;
}

// 在 Windows 系统上运行这段代码时，会注意到第一行的第一个单词损坏了。
// 这是因为 Windows总是在任何 UTF-8 文件的开头包含一个字节顺序标记 (BOM)
void skip_bom(auto& fs) {
    const unsigned char boms[]{0xef, 0xbb, 0xbf};
    bool have_bom{true};
    for (const auto& c : boms) {
        if ((unsigned char)fs.get() != c) {
            have_bom = false;
        }
    }
    if (!have_bom) {
        fs.seekg(0);
    }
    return;
}

TEST(StringTest, ReadStructTest) {
    std::ifstream input_file("cpp20_stl/resource/cities.txt");
    EXPECT_TRUE(input_file.is_open());
    skip_bom(input_file);

    std::vector<CityInfo> cities;
    CityInfo city{};

    for (CityInfo city{}; input_file >> city;) {
        cities.push_back(city);
    }

    EXPECT_EQ(cities, (std::vector<CityInfo>{
                          {"Las Vegas", 661903, 36.1699, -115.1398},
                          {"New York City", 8850000, 40.7128, -74.0060},
                          {"Berlin", 3571000, 52.5200, 13.4050},
                          {"Mexico City", 21900000, 19.4326, -99.1332},
                          {"Sydney", 5312000, -33.8688, 151.2093},
                      }));
}

#pragma endregion

#pragma region 用 char_traits 定义一个字符串类

constexpr char char_lower(const char& c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return c;
}

// 大小无关字符特征类
class ci_traits : public std::char_traits<char> {
public:
    static constexpr bool eq(const char_type& lhs, const char_type& rhs) noexcept {
        return char_lower(lhs) == char_lower(rhs);
    }

    static constexpr bool lt(const char_type& lhs, const char_type& rhs) noexcept {
        return char_lower(lhs) < char_lower(rhs);
    }

    static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) {
        for (size_t i = 0; i < n; ++i) {
            if (lt(s1[i], s2[i])) {
                return -1;
            }
            if (lt(s2[i], s1[i])) {
                return 1;
            }
        }
        return 0;
    }

    static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) {
        const char_type find_a = char_lower(a);
        for (size_t i = 0; i < n; ++i) {
            if (char_lower(s[i]) == find_a) {
                return s + i;
            }
        }
        return nullptr;
    }
};

using ci_string = std::basic_string<char, ci_traits>;

std::ostream& operator<<(std::ostream& os, const ci_string& str) { return os << str.c_str(); }

// 重写 assign() 和 copy() 成员函数来创建一个存储小写字符的类
class lc_traits : public std::char_traits<char> {
public:
    static constexpr void assign(char_type& c1, const char_type& c2) noexcept { c1 = char_lower(c2); }

    static constexpr char_type* assign(char_type* s, size_t n, char_type a) {
        for (size_t i = 0; i < n; ++i) {
            s[i] = char_lower(a);
        }
        return s;
    }

    static constexpr char_type* copy(char_type* s1, const char_type* s2, size_t n) {
        for (size_t i = 0; i < n; ++i) {
            s1[i] = char_lower(s2[i]);
        }
        return s1;
    }
};

using lc_string = std::basic_string<char, lc_traits>;

TEST(StringTest, CharTraitsTest) {
    ci_string ci_s{"Foo Bar Baz"};
    EXPECT_EQ(fmt::format("{}", ci_s), "Foo Bar Baz");
    EXPECT_EQ(ci_s.find('b'), 4);

    ci_string copmare1{"CoMpArE StRiNg"};
    ci_string compare2{"compare string"};
    EXPECT_EQ(copmare1, compare2);

    lc_string lc_s{"Foo Bar Baz"};
    EXPECT_EQ(fmt::format("{}", lc_s), "foo bar baz");
}

#pragma endregion

#pragma region 用正则表达式解析字符串

TEST(StringTest, RegexTest) {
    // I. 匹配整个字符串。
    // II. 查找子字符串 <a href=”
    // III. 存储到下一个双引号间的所有内容，作为子匹配 1。
    // IV. 跳过 > 字符。
    // V. 将字符串 </a> 之前的所有内容存储为子匹配 2。
    const std::regex link_re{"<a href=\"([^\"]*)\"[^<]*>([^<]*)</a>"};

    std::ifstream infile("cpp20_stl/resource/the-end.html", std::ios_base::in);

    std::string in;
    for (std::string line{}; std::getline(infile, line);) {
        in += line;
    }

    std::vector<std::string> result;

    // sregex_token_iterator 支持子匹配, 1 和 2 对应正则表达式中的子匹配项
    std::sregex_token_iterator it{in.begin(), in.end(), link_re, {1, 2}};
    for (std::sregex_token_iterator end_it{}; it != end_it;) {
        const std::string link{*it++};
        if (it == end_it) {
            break;
        }
        const std::string desc{*it++};
        result.push_back(fmt::format("{:.<24} {}", desc, link));
    }
    EXPECT_EQ(result, (std::vector<std::string>{
                          "Bill Weinman............ https://bw.org/",
                          "courses................. https://bw.org/courses/",
                          "music................... https://bw.org/music/",
                          "books................... https://packt.com/",
                          "back to the internet.... https://duckduckgo.com/",
                      }));
}

#pragma endregion

}  // namespace pyc
