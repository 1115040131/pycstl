#include <fstream>
#include <map>
#include <ranges>
#include <regex>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "common/cin_redirect.h"

namespace pyc {

inline constexpr std::string_view re{"(\\w+)"};

void word_count() {
    std::map<std::string, int> word_map{};
    std::vector<std::pair<std::string, int>> word_vec{};
    std::regex word_re(re.data());
    size_t total_words{};

    // 统计单词数量
    for (std::string s{}; std::cin >> s;) {
        auto words_begin{std::sregex_iterator(s.begin(), s.end(), word_re)};
        auto words_end{std::sregex_iterator()};
        for (auto iter{words_begin}; iter != words_end; ++iter) {
            std::smatch match{*iter};
            auto word_str(match.str());
            std::ranges::transform(word_str, word_str.begin(), [](char c) { return std::tolower(c); });
            auto [map_iter, result] = word_map.try_emplace(word_str, 0);
            auto& [word, count] = *map_iter;
            ++count;
            ++total_words;
        }
    }

    // 按词频降序排列
    auto unique_words = word_map.size();
    word_vec.reserve(unique_words);
    std::ranges::move(word_map, std::back_inserter(word_vec));
    std::ranges::sort(word_vec, [](const auto& lhs, const auto& rhs) {
        if (lhs.second != rhs.second) {
            return lhs.second > rhs.second;
        }
        return lhs.first < rhs.first;
    });

    fmt::println("Total words: {}", total_words);
    fmt::println("Unique words: {}", unique_words);
    for (int limit{20}; auto& [word, count] : word_vec) {
        fmt::println("{}: {}", count, word);
        if (--limit == 0) {
            break;
        }
    }
}

TEST(WordCounterTest, WordCounterTest) {
    std::ifstream input_file("cpp20_stl/resource/the-raven.txt");
    EXPECT_TRUE(input_file.is_open());

    CinRedirect cin_redirect(input_file);
    word_count();
}

}  // namespace pyc
