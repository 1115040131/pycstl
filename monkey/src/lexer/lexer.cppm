module;

#include <cstddef>
#include <memory>
#include <string_view>

export module monkey.lexer;

export import monkey.token;

export namespace pyc::monkey {

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
    std::string_view input_;
    size_t position_{};
};

}  // namespace pyc::monkey
