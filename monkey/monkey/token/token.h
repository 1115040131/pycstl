#pragma once

#include <map>
#include <string_view>

namespace pyc {
namespace monkey {

struct Token {
    enum class Type {
        kIllegal,
        kEof,

        // Identifiers + literals
        kIdent,  // add, foobar, x, y, ...
        kInt,    // 123456

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

        kLParen,  // (
        kRParen,  // )
        kLBrace,  // {
        kRBrace,  // }

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

std::string_view toString(Token::Type type) {
    switch (type) {
        case Token::Type::kIllegal:
            return "ILLEGAL";
        case Token::Type::kEof:
            return "EOF";
        case Token::Type::kIdent:
            return "IDENT";
        case Token::Type::kInt:
            return "INT";
        case Token::Type::kAssign:
            return "=";
        case Token::Type::kPlus:
            return "+";
        case Token::Type::kMinus:
            return "-";
        case Token::Type::kBang:
            return "!";
        case Token::Type::kAsterisk:
            return "*";
        case Token::Type::kSlash:
            return "/";
        case Token::Type::kLt:
            return "<";
        case Token::Type::kGt:
            return ">";
        case Token::Type::kEq:
            return "==";
        case Token::Type::kNotEq:
            return "!=";
        case Token::Type::kComma:
            return ",";
        case Token::Type::kSemicolon:
            return ";";
        case Token::Type::kLParen:
            return "(";
        case Token::Type::kRParen:
            return ")";
        case Token::Type::kLBrace:
            return "{";
        case Token::Type::kRBrace:
            return "}";
        case Token::Type::kFunction:
            return "FUNCTION";
        case Token::Type::kLet:
            return "LET";
        case Token::Type::kTrue:
            return "TRUE";
        case Token::Type::kFalse:
            return "FALSE";
        case Token::Type::kIf:
            return "IF";
        case Token::Type::kElse:
            return "ELSE";
        case Token::Type::kReturn:
            return "RETURN";
        default:
            return "UNKNOWN";
    }
}

}  // namespace monkey
}  // namespace pyc
