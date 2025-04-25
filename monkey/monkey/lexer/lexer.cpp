#include "monkey/lexer/lexer.h"

#include <fmt/base.h>

namespace pyc {
namespace monkey {

std::unique_ptr<Lexer> Lexer::New(std::string_view input) { return std::make_unique<Lexer>(input); }

Token Lexer::nextToken() {
    skipWhitespace();

    Token token;

    auto begin = position_;
    auto ch = readChar();

    switch (ch) {
        case '=':
            if (peekChar() == '=') {
                position_++;
                token.type = Token::Type::kEq;
            } else {
                token.type = Token::Type::kAssign;
            }
            break;
        case '+':
            token.type = Token::Type::kPlus;
            break;
        case '-':
            token.type = Token::Type::kMinus;
            break;
        case '!':
            if (peekChar() == '=') {
                position_++;
                token.type = Token::Type::kNotEq;
            } else {
                token.type = Token::Type::kBang;
            }
            break;
        case '*':
            token.type = Token::Type::kAsterisk;
            break;
        case '/':
            token.type = Token::Type::kSlash;
            break;
        case '<':
            token.type = Token::Type::kLt;
            break;
        case '>':
            token.type = Token::Type::kGt;
            break;
        case ',':
            token.type = Token::Type::kComma;
            break;
        case ';':
            token.type = Token::Type::kSemicolon;
            break;
        case '(':
            token.type = Token::Type::kLParen;
            break;
        case ')':
            token.type = Token::Type::kRParen;
            break;
        case '{':
            token.type = Token::Type::kLBrace;
            break;
        case '}':
            token.type = Token::Type::kRBrace;
            break;
        case '\0':
            token.type = Token::Type::kEof;
            break;
        default:
            if (std::isalpha(ch)) {
                while (std::isalpha(peekChar()) || std::isdigit(peekChar())) {
                    position_++;
                }
                auto iter = kKeywords.find(input_.substr(begin, position_ - begin));
                if (iter != kKeywords.end()) {
                    token.type = iter->second;
                } else {
                    token.type = Token::Type::kIdent;
                }
            } else if (std::isdigit(ch)) {
                while (std::isdigit(peekChar())) {
                    position_++;
                }
                token.type = Token::Type::kInt;
            } else {
                token.type = Token::Type::kIllegal;
            }
            break;
    }
    if (token.type == Token::Type::kEof) {
        token.literal = "";
    } else {
        token.literal = std::string_view(input_.data() + begin, position_ - begin);
    }

    return token;
}

char Lexer::peekChar() {
    if (position_ < input_.size()) {
        return input_[position_];
    }
    return 0;
}

char Lexer::readChar() {
    auto ch = peekChar();
    position_++;
    return ch;
}

void Lexer::skipWhitespace() {
    while (true) {
        auto ch = peekChar();
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            position_++;
        } else {
            break;
        }
    }
}

}  // namespace monkey
}  // namespace pyc