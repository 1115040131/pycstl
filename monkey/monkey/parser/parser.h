#pragma once

#include <unordered_map>
#include <vector>

#include "monkey/ast/ast.h"
#include "monkey/lexer/lexer.h"

namespace pyc {
namespace monkey {

class Parser {
public:
    static std::unique_ptr<Parser> New(std::unique_ptr<Lexer> lexer);

    Parser();

    std::unique_ptr<Program> parseProgram();

    const std::vector<std::string>& errors() const { return errors_; }

    std::string errorsToString() const {
        std::string result;
        for (const auto& error : errors_) {
            result += error + "\n";
        }
        return result;
    }

private:
    enum class Priority {
        LOWEST = 1,
        EQUALS,       // ==
        LESSGREATER,  // > or <
        SUM,          // +
        PRODUCT,      // *
        PREFIX,       // -X or !X
        CALL,         // myFunction(X)
    };

    void nextToken();

    bool expectPeek(Token::Type type);

    void noPrefixParseFnError(Token::Type type);

    // 解析语句
    std::unique_ptr<Statement> parseStatement();

    // 解析表达式
    std::unique_ptr<Expression> parseExpression(Priority precedence);

    std::unique_ptr<LetStatement> parseLetStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<Statement> parseExpressionStatement();
    std::unique_ptr<BlockStatement> parseBlockStatement();

    // 解析词法单元
    std::unique_ptr<Expression> parseGroupedExpression();
    std::unique_ptr<Expression> parseIdentifier();
    std::unique_ptr<Expression> parseBoolean();
    std::unique_ptr<Expression> parseIntegerLiteral();
    std::unique_ptr<Expression> parsePrefixExpression();
    std::unique_ptr<Expression> parseInfixExpression(std::unique_ptr<Expression> left);
    std::unique_ptr<Expression> parseIfExpression();
    std::unique_ptr<Expression> parseFunctionLiteral();
    std::unique_ptr<Expression> parseCallExpression(std::unique_ptr<Expression> function);

    std::vector<std::unique_ptr<Identifier>> parseFunctionParameters();
    std::vector<std::unique_ptr<Expression>> parseCallArguments();

private:
    std::unique_ptr<Lexer> lexer_;
    std::vector<std::string> errors_;

    Token current_token_;
    Token peek_token_;

    using PrefixParseFn = std::unique_ptr<Expression> (Parser::*)();
    using InfixParseFn = std::unique_ptr<Expression> (Parser::*)(std::unique_ptr<Expression>);

    std::unordered_map<Token::Type, PrefixParseFn> prefix_parse_fns_;
    std::unordered_map<Token::Type, InfixParseFn> infix_parse_fns_;

    std::unordered_map<Token::Type, Priority> precedences_ = {
        {Token::Type::kEq, Priority::EQUALS},      {Token::Type::kNotEq, Priority::EQUALS},
        {Token::Type::kLt, Priority::LESSGREATER}, {Token::Type::kGt, Priority::LESSGREATER},
        {Token::Type::kPlus, Priority::SUM},       {Token::Type::kMinus, Priority::SUM},
        {Token::Type::kSlash, Priority::PRODUCT},  {Token::Type::kAsterisk, Priority::PRODUCT},
        {Token::Type::kLParen, Priority::CALL},
    };
};

}  // namespace monkey
}  // namespace pyc