#include <fmt/base.h>
#include <gtest/gtest.h>

#include "monkey/lexer/lexer.h"
#include "monkey/test/test_define.h"

namespace pyc {
namespace monkey {

TEST(LexerTest, ToStringTest) {
    EXPECT_EQ(std::string(toString(Token::Type::kIllegal)), "ILLEGAL");
    EXPECT_EQ(std::string(toString(Token::Type::kEof)), "EOF");
    EXPECT_EQ(std::string(toString(Token::Type::kIdent)), "IDENT");
    EXPECT_EQ(std::string(toString(Token::Type::kInt)), "INT");
}

TEST(LexerTest, NextTokenTest) {
    std::string input = R""(
        let five = 5;
        let ten = 10;

        let add = fn(x, y) {
             x + y;
        };

        let result = add(five, ten);
        !-/*5;
        5 < 10 > 5;

        if (5 < 10) {
            return true;
        } else {
            return false;
        }

        10 == 10;
        10 != 9;
        "foobar"
        "foo bar"
        [1, 2];
        {"foo": "bar"}
    )"";

    std::vector<Token> tests{
        {Token::Type::kLet, "let"},        {Token::Type::kIdent, "five"},    {Token::Type::kAssign, "="},
        {Token::Type::kInt, "5"},          {Token::Type::kSemicolon, ";"},

        {Token::Type::kLet, "let"},        {Token::Type::kIdent, "ten"},     {Token::Type::kAssign, "="},
        {Token::Type::kInt, "10"},         {Token::Type::kSemicolon, ";"},

        {Token::Type::kLet, "let"},        {Token::Type::kIdent, "add"},     {Token::Type::kAssign, "="},
        {Token::Type::kFunction, "fn"},    {Token::Type::kLParen, "("},      {Token::Type::kIdent, "x"},
        {Token::Type::kComma, ","},        {Token::Type::kIdent, "y"},       {Token::Type::kRParen, ")"},
        {Token::Type::kLBrace, "{"},       {Token::Type::kIdent, "x"},       {Token::Type::kPlus, "+"},
        {Token::Type::kIdent, "y"},        {Token::Type::kSemicolon, ";"},   {Token::Type::kRBrace, "}"},
        {Token::Type::kSemicolon, ";"},

        {Token::Type::kLet, "let"},        {Token::Type::kIdent, "result"},  {Token::Type::kAssign, "="},
        {Token::Type::kIdent, "add"},      {Token::Type::kLParen, "("},      {Token::Type::kIdent, "five"},
        {Token::Type::kComma, ","},        {Token::Type::kIdent, "ten"},     {Token::Type::kRParen, ")"},
        {Token::Type::kSemicolon, ";"},

        {Token::Type::kBang, "!"},         {Token::Type::kMinus, "-"},       {Token::Type::kSlash, "/"},
        {Token::Type::kAsterisk, "*"},     {Token::Type::kInt, "5"},         {Token::Type::kSemicolon, ";"},

        {Token::Type::kInt, "5"},          {Token::Type::kLt, "<"},          {Token::Type::kInt, "10"},
        {Token::Type::kGt, ">"},           {Token::Type::kInt, "5"},         {Token::Type::kSemicolon, ";"},

        {Token::Type::kIf, "if"},          {Token::Type::kLParen, "("},      {Token::Type::kInt, "5"},
        {Token::Type::kLt, "<"},           {Token::Type::kInt, "10"},        {Token::Type::kRParen, ")"},
        {Token::Type::kLBrace, "{"},       {Token::Type::kReturn, "return"}, {Token::Type::kTrue, "true"},
        {Token::Type::kSemicolon, ";"},    {Token::Type::kRBrace, "}"},      {Token::Type::kElse, "else"},
        {Token::Type::kLBrace, "{"},       {Token::Type::kReturn, "return"}, {Token::Type::kFalse, "false"},
        {Token::Type::kSemicolon, ";"},    {Token::Type::kRBrace, "}"},

        {Token::Type::kInt, "10"},         {Token::Type::kEq, "=="},         {Token::Type::kInt, "10"},
        {Token::Type::kSemicolon, ";"},

        {Token::Type::kInt, "10"},         {Token::Type::kNotEq, "!="},      {Token::Type::kInt, "9"},
        {Token::Type::kSemicolon, ";"},

        {Token::Type::kString, "foobar"},

        {Token::Type::kString, "foo bar"},

        {Token::Type::kLBracket, "["},     {Token::Type::kInt, "1"},         {Token::Type::kComma, ","},
        {Token::Type::kInt, "2"},          {Token::Type::kRBracket, "]"},    {Token::Type::kSemicolon, ";"},

        {Token::Type::kLBrace, "{"},       {Token::Type::kString, "foo"},    {Token::Type::kColon, ":"},
        {Token::Type::kString, "bar"},     {Token::Type::kRBrace, "}"},

        {Token::Type::kEof, ""},
    };

    auto lexer = Lexer::New(input);
    for (const auto& test : tests) {
        EXPECT_EQ(test, lexer->nextToken());
    }
}

}  // namespace monkey
}  // namespace pyc
