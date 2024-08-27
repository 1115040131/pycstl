#include <algorithm>
#include <fstream>
#include <string_view>
#include <vector>

#include <fmt/base.h>
#include <gtest/gtest.h>

#include "common/cin_redirect.h"

namespace pyc {

/// @brief 是否到句子末尾
bool is_eos(std::string_view str) {
    constexpr std::string_view eos{".!?"};
    for (auto c : str) {
        if (eos.find(c) != std::string_view::npos) {
            return true;
        }
    }
    return false;
}

void long_sentence() {
    std::vector<std::vector<std::string>> sentences;
    sentences.emplace_back();
    for (std::string s{}; std::cin >> s;) {
        sentences.back().emplace_back(s);
        if (is_eos(s)) {
            sentences.emplace_back();
        }
    }
    if (sentences.back().empty()) {
        sentences.pop_back();
    }

    std::ranges::sort(sentences, [](const std::vector<std::string>& lhs, const std::vector<std::string>& rhs) {
        return lhs.size() > rhs.size();
    });

    constexpr int kLimit = 10;
    for (const auto& sentence : sentences) {
        size_t size = sentence.size();
        size_t limit{kLimit};
        fmt::print("{}: ", size);
        for (const auto& word : sentence) {
            fmt::print("{} ", word);
            if (--limit == 0) {
                if (size > kLimit) {
                    fmt::print("...");
                }
                break;
            }
        }
        fmt::print("\n");
    }
}

TEST(LongSentenceTest, LongSentenceTest) {
    std::ifstream input_file("cpp20_stl/resource/sentences.txt");
    CinRedirect cin_redirect(input_file);
    long_sentence();
}

}  // namespace pyc
