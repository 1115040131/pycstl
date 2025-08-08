#pragma once

#include <map>
#include <string_view>

#include <fmt/core.h>
#include <fmt/format.h>

#include "monkey/macro.h"

namespace pyc {
namespace monkey {

struct Token {
    enum class Type {
        kIllegal,
        kEof,

        // Identifiers + literals
        kIdent,   // add, foobar, x, y, ...
        kInt,     // 123456
        kString,  // "hello world"

        // Operators
        kAssign,    // =
        kPlus,      // +
        kMinus,     // -
        kBang,      // !
        kAsterisk,  // *
        kSlash,     // /

        kLt,  // <
        kGt,  // >

        kEq,     // ==
        kNotEq,  // !=

        // Delimiters
        kComma,      // ,
        kSemicolon,  // ;
        kColon,      // :

        kLParen,  // (
        kRParen,  // )
        kLBrace,  // {
        kRBrace,  // }

        kLBracket,  // [
        kRBracket,  // ]

        // Keywords
        kFunction,  // fn
        kLet,       // let
        kTrue,      // true
        kFalse,     // false
        kIf,        // if
        kElse,      // else
        kReturn,    // return
    };

    Type type;
    std::string_view literal;
};

static const std::map<std::string_view, Token::Type> kKeywords{
    {"fn", Token::Type::kFunction},   {"let", Token::Type::kLet}, {"true", Token::Type::kTrue},
    {"false", Token::Type::kFalse},   {"if", Token::Type::kIf},   {"else", Token::Type::kElse},
    {"return", Token::Type::kReturn},
};

inline constexpr std::string_view toString(Token::Type type) {
    switch (type) {
        TO_STRING_CASE1(Token::Type, kIllegal, "ILLEGAL");
        TO_STRING_CASE1(Token::Type, kEof, "EOF");
        TO_STRING_CASE1(Token::Type, kIdent, "IDENT");
        TO_STRING_CASE1(Token::Type, kInt, "INT");
        TO_STRING_CASE1(Token::Type, kString, "STRING");
        TO_STRING_CASE1(Token::Type, kAssign, "=");
        TO_STRING_CASE1(Token::Type, kPlus, "+");
        TO_STRING_CASE1(Token::Type, kMinus, "-");
        TO_STRING_CASE1(Token::Type, kBang, "!");
        TO_STRING_CASE1(Token::Type, kAsterisk, "*");
        TO_STRING_CASE1(Token::Type, kSlash, "/");
        TO_STRING_CASE1(Token::Type, kLt, "<");
        TO_STRING_CASE1(Token::Type, kGt, ">");
        TO_STRING_CASE1(Token::Type, kEq, "==");
        TO_STRING_CASE1(Token::Type, kNotEq, "!=");
        TO_STRING_CASE1(Token::Type, kComma, ",");
        TO_STRING_CASE1(Token::Type, kSemicolon, ";");
        TO_STRING_CASE1(Token::Type, kLParen, "(");
        TO_STRING_CASE1(Token::Type, kRParen, ")");
        TO_STRING_CASE1(Token::Type, kLBrace, "{");
        TO_STRING_CASE1(Token::Type, kRBrace, "}");
        TO_STRING_CASE1(Token::Type, kLBracket, "[");
        TO_STRING_CASE1(Token::Type, kRBracket, "]");
        TO_STRING_CASE1(Token::Type, kFunction, "FUNCTION");
        TO_STRING_CASE1(Token::Type, kLet, "LET");
        TO_STRING_CASE1(Token::Type, kTrue, "TRUE");
        TO_STRING_CASE1(Token::Type, kFalse, "FALSE");
        TO_STRING_CASE1(Token::Type, kIf, "IF");
        TO_STRING_CASE1(Token::Type, kElse, "ELSE");
        TO_STRING_CASE1(Token::Type, kReturn, "RETURN");
        default:
            return "UNKNOWN";
    }
}

}  // namespace monkey
}  // namespace pyc

namespace fmt {

template <>
struct formatter<pyc::monkey::Token> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const pyc::monkey::Token& token, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "Token{{type: \"{}\", literal: \"{}\"}}",
                              pyc::monkey::toString(token.type), token.literal);
    }
};

}  // namespace fmt