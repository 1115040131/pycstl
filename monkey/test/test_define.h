#pragma once

#include <iostream>

#include "monkey/parser/parser.h"
#include "monkey/token/token.h"

namespace pyc {
namespace monkey {

inline bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.literal == rhs.literal;
}

inline std::ostream& operator<<(std::ostream& os, const Token& token) {
    return os << "Token{type: \"" << toString(token.type) << "\", literal: \"" << token.literal << "\"}";
}

#define TEST_NULL_OBJECT(object, input)                                                                     \
    {                                                                                                       \
        auto null = std::dynamic_pointer_cast<Null>(object);                                                \
        ASSERT_TRUE(null != nullptr) << "Input: " << input << "\nExpected Null, got " << object->typeStr(); \
    }

#define TEST_INTEGER_OBJECT(object, expected, input)                                                              \
    {                                                                                                             \
        auto integer = std::dynamic_pointer_cast<Integer>(object);                                                \
        ASSERT_TRUE(integer != nullptr) << "Input: " << input << "\nExpected Integer, got " << object->typeStr(); \
        EXPECT_EQ(integer->value(), expected) << "Input: " << input;                                              \
    }

#define TEST_BOOLEAN_OBJECT(object, expected, input)                                        \
    {                                                                                       \
        auto boolean = std::dynamic_pointer_cast<BooleanObject>(object);                    \
        ASSERT_TRUE(boolean != nullptr)                                                     \
            << "Input: " << input << "\nExpected BooleanObject, got " << object->typeStr(); \
        EXPECT_EQ(boolean->value(), expected) << "Input: " << input;                        \
    }

#define TEST_STRING_OBJECT(object, expected, input)                                                          \
    {                                                                                                        \
        auto str = std::dynamic_pointer_cast<String>(object);                                                \
        ASSERT_TRUE(str != nullptr) << "Input: " << input << "\nExpected String, got " << object->typeStr(); \
        EXPECT_EQ(str->value(), expected) << "Input: " << input;                                             \
    }

#define TEST_EXPECTED_OBJECT(object, expected, input)                    \
    {                                                                    \
        if (std::holds_alternative<int>(expected)) {                     \
            TEST_INTEGER_OBJECT(object, std::get<int>(expected), input); \
        }                                                                \
    }

inline std::unique_ptr<Node> processInput(std::string_view input) {
    auto lexer = Lexer::New(input);
    auto parser = Parser::New(std::move(lexer));
    auto program = parser->parseProgram();
    if (parser->errors().size() > 0) {
        std::cerr << "Parser errors: " << parser->errorsToString() << std::endl;
        return nullptr;
    }
    return program;
}

}  // namespace monkey
}  // namespace pyc