#include "monkey/parser/parser.h"

namespace pyc {
namespace monkey {

std::unique_ptr<Parser> Parser::New(std::unique_ptr<Lexer> lexer) {
    auto parser = std::make_unique<Parser>();
    parser->lexer_ = std::move(lexer);
    return parser;
}

Parser::Parser() {
    prefix_parse_fns_.emplace(Token::Type::kIdent, &Parser::parseIdentifier);
    prefix_parse_fns_.emplace(Token::Type::kInt, &Parser::parseIntegerLiteral);
    prefix_parse_fns_.emplace(Token::Type::kString, &Parser::parseStringLiteral);
    prefix_parse_fns_.emplace(Token::Type::kLBracket, &Parser::parseArrayLiteral);
    prefix_parse_fns_.emplace(Token::Type::kBang, &Parser::parsePrefixExpression);
    prefix_parse_fns_.emplace(Token::Type::kMinus, &Parser::parsePrefixExpression);
    prefix_parse_fns_.emplace(Token::Type::kTrue, &Parser::parseBoolean);
    prefix_parse_fns_.emplace(Token::Type::kFalse, &Parser::parseBoolean);
    prefix_parse_fns_.emplace(Token::Type::kLParen, &Parser::parseGroupedExpression);
    prefix_parse_fns_.emplace(Token::Type::kIf, &Parser::parseIfExpression);
    prefix_parse_fns_.emplace(Token::Type::kFunction, &Parser::parseFunctionLiteral);

    infix_parse_fns_.emplace(Token::Type::kPlus, &Parser::parseInfixExpression);
    infix_parse_fns_.emplace(Token::Type::kMinus, &Parser::parseInfixExpression);
    infix_parse_fns_.emplace(Token::Type::kSlash, &Parser::parseInfixExpression);
    infix_parse_fns_.emplace(Token::Type::kAsterisk, &Parser::parseInfixExpression);
    infix_parse_fns_.emplace(Token::Type::kEq, &Parser::parseInfixExpression);
    infix_parse_fns_.emplace(Token::Type::kNotEq, &Parser::parseInfixExpression);
    infix_parse_fns_.emplace(Token::Type::kLt, &Parser::parseInfixExpression);
    infix_parse_fns_.emplace(Token::Type::kGt, &Parser::parseInfixExpression);
    infix_parse_fns_.emplace(Token::Type::kLParen, &Parser::parseCallExpression);
    infix_parse_fns_.emplace(Token::Type::kLBracket, &Parser::parseIndexExpression);
}

void Parser::nextToken() {
    current_token_ = peek_token_;
    peek_token_ = lexer_->nextToken();
}

bool Parser::expectPeek(Token::Type type) {
    if (peek_token_.type == type) {
        nextToken();
        return true;
    }
    errors_.push_back(
        fmt::format("expecte next token to be {}, got {} instead", toString(type), toString(peek_token_.type)));
    return false;
}

void Parser::noPrefixParseFnError(Token::Type type) {
    errors_.push_back(fmt::format("no prefix parse function for {} found", toString(type)));
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    current_token_ = lexer_->nextToken();
    peek_token_ = lexer_->nextToken();

    while (current_token_.type != Token::Type::kEof) {
        auto statement = parseStatement();
        if (statement) {
            program->addStatement(std::move(statement));
        }
        nextToken();
    }
    return program;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    if (current_token_.type == Token::Type::kLet) {
        return parseLetStatement();
    } else if (current_token_.type == Token::Type::kReturn) {
        return parseReturnStatement();
    }
    return parseExpressionStatement();
}

std::unique_ptr<Expression> Parser::parseExpression(Priority precedence) {
    auto prefix_parse_fn = prefix_parse_fns_.find(current_token_.type);
    if (prefix_parse_fn == prefix_parse_fns_.end()) {
        noPrefixParseFnError(current_token_.type);
        return nullptr;
    }

    auto left_expression = (this->*(prefix_parse_fn->second))();
    while (peek_token_.type != Token::Type::kSemicolon && precedence < precedences_[peek_token_.type]) {
        auto infix_parse_fn = infix_parse_fns_.find(peek_token_.type);
        if (infix_parse_fn == infix_parse_fns_.end()) {
            return left_expression;
        }
        nextToken();
        left_expression = (this->*(infix_parse_fn->second))(std::move(left_expression));
    }
    return left_expression;
}

std::unique_ptr<LetStatement> Parser::parseLetStatement() {
    auto statement = std::make_unique<LetStatement>(current_token_);
    if (!expectPeek(Token::Type::kIdent)) {
        return nullptr;
    }
    statement->setName(std::make_unique<Identifier>(current_token_));
    if (!expectPeek(Token::Type::kAssign)) {
        return nullptr;
    }
    nextToken();
    statement->setValue(parseExpression(Priority::LOWEST));

    if (peek_token_.type == Token::Type::kSemicolon) {
        nextToken();
    }

    return statement;
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
    auto statement = std::make_unique<ReturnStatement>(current_token_);
    nextToken();
    statement->setValue(parseExpression(Priority::LOWEST));
    if (peek_token_.type == Token::Type::kSemicolon) {
        nextToken();
    }
    return statement;
}

std::unique_ptr<Statement> Parser::parseExpressionStatement() {
    auto statement = std::make_unique<ExpressionStatement>(current_token_);
    statement->setExpression(parseExpression(Priority::LOWEST));
    if (peek_token_.type == Token::Type::kSemicolon) {
        nextToken();
    }
    return statement;
}

std::unique_ptr<BlockStatement> Parser::parseBlockStatement() {
    auto block = std::make_unique<BlockStatement>(current_token_);
    nextToken();
    while (current_token_.type != Token::Type::kRBrace && current_token_.type != Token::Type::kEof) {
        auto statement = parseStatement();
        if (statement) {
            block->addStatement(std::move(statement));
        }
        nextToken();
    }
    return block;
}

std::unique_ptr<Expression> Parser::parseGroupedExpression() {
    nextToken();
    auto expression = parseExpression(Priority::LOWEST);
    if (!expectPeek(Token::Type::kRParen)) {
        return nullptr;
    }
    return expression;
}

std::unique_ptr<Expression> Parser::parseIdentifier() { return std::make_unique<Identifier>(current_token_); }

std::unique_ptr<Expression> Parser::parseBoolean() {
    return std::make_unique<Boolean>(current_token_, current_token_.type == Token::Type::kTrue);
}

std::unique_ptr<Expression> Parser::parseIntegerLiteral() {
    auto literal = std::make_unique<IntegerLiteral>(current_token_);
    if (current_token_.literal == "0") {
        literal->setValue(0);
    } else {
        try {
            literal->setValue(std::stoll(current_token_.literal.data()));
        } catch (const std::invalid_argument&) {
            errors_.push_back(fmt::format("could not parse {} as integer", current_token_.literal));
            return nullptr;
        }
    }
    return literal;
}

std::unique_ptr<Expression> Parser::parseStringLiteral() {
    return std::make_unique<StringLiteral>(current_token_);
}

std::unique_ptr<Expression> Parser::parseArrayLiteral() {
    auto array = std::make_unique<ArrayLiteral>(current_token_);
    array->setElements(parseExpressionList(Token::Type::kRBracket));
    return array;
}

std::unique_ptr<Expression> Parser::parseIndexExpression(std::unique_ptr<Expression> left) {
    auto expression = std::make_unique<IndexExpression>(current_token_, std::move(left));
    nextToken();
    expression->setIndex(parseExpression(Priority::LOWEST));
    if (!expectPeek(Token::Type::kRBracket)) {
        return nullptr;
    }
    return expression;
}

std::unique_ptr<Expression> Parser::parsePrefixExpression() {
    auto expression = std::make_unique<PrefixExpression>(current_token_);
    nextToken();
    expression->setRight(parseExpression(Priority::PREFIX));
    return expression;
}

std::unique_ptr<Expression> Parser::parseInfixExpression(std::unique_ptr<Expression> left) {
    auto expression = std::make_unique<InfixExpression>(current_token_, std::move(left));
    auto precedence = precedences_[current_token_.type];
    nextToken();
    expression->setRight(parseExpression(precedence));
    return expression;
}

std::unique_ptr<Expression> Parser::parseIfExpression() {
    auto expression = std::make_unique<IfExpression>(current_token_);
    if (!expectPeek(Token::Type::kLParen)) {
        return nullptr;
    }
    nextToken();
    expression->setCondition(parseExpression(Priority::LOWEST));
    if (!expectPeek(Token::Type::kRParen)) {
        return nullptr;
    }
    if (!expectPeek(Token::Type::kLBrace)) {
        return nullptr;
    }
    expression->setConsequence(parseBlockStatement());
    if (peek_token_.type == Token::Type::kElse) {
        nextToken();
        if (!expectPeek(Token::Type::kLBrace)) {
            return nullptr;
        }
        expression->setAlternative(parseBlockStatement());
    }
    return expression;
}

std::unique_ptr<Expression> Parser::parseFunctionLiteral() {
    auto function = std::make_unique<FunctionLiteral>(current_token_);
    if (!expectPeek(Token::Type::kLParen)) {
        return nullptr;
    }
    function->setParameters(parseFunctionParameters());
    if (!expectPeek(Token::Type::kLBrace)) {
        return nullptr;
    }
    function->setBody(parseBlockStatement());
    return function;
}

std::unique_ptr<Expression> Parser::parseCallExpression(std::unique_ptr<Expression> function) {
    auto expression = std::make_unique<CallExpression>(current_token_, std::move(function));
    expression->setArguments(parseExpressionList(Token::Type::kRParen));
    return expression;
}

std::vector<std::shared_ptr<Identifier>> Parser::parseFunctionParameters() {
    std::vector<std::shared_ptr<Identifier>> parameters;
    if (peek_token_.type == Token::Type::kRParen) {
        nextToken();
        return parameters;
    }
    if (!expectPeek(Token::Type::kIdent)) {
        return {};
    }
    parameters.emplace_back(std::make_unique<Identifier>(current_token_));
    while (peek_token_.type == Token::Type::kComma) {
        nextToken();
        if (!expectPeek(Token::Type::kIdent)) {
            return {};
        }
        parameters.emplace_back(std::make_unique<Identifier>(current_token_));
    }
    if (!expectPeek(Token::Type::kRParen)) {
        return {};
    }
    return parameters;
}

std::vector<std::shared_ptr<Expression>> Parser::parseExpressionList(Token::Type end_token) {
    std::vector<std::shared_ptr<Expression>> list;
    nextToken();
    if (current_token_.type == end_token) {
        return list;
    }
    list.emplace_back(parseExpression(Priority::LOWEST));
    while (peek_token_.type == Token::Type::kComma) {
        nextToken();
        nextToken();
        list.emplace_back(parseExpression(Priority::LOWEST));
    }
    if (!expectPeek(end_token)) {
        return {};
    }
    return list;
}

}  // namespace monkey
}  // namespace pyc