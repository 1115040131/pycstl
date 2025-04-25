#pragma once

#include <memory>
#include <string>

#include "monkey/token/token.h"

namespace pyc {
namespace monkey {

class Lexer {
public:
    static std::unique_ptr<Lexer> New(std::string_view input);

    Lexer(std::string_view input) : input_(input) {}

    Token nextToken();

private:
    // 获取当前字符
    char peekChar();

    // 读取一个字符
    char readChar();

    void skipWhitespace();

private:
    std::string input_;
    size_t position_{};
};

}  // namespace monkey
}  // namespace pyc