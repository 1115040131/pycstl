#include "monkey/ast/ast.h"

#include <sstream>

namespace pyc {
namespace monkey {

std::string_view toString(Node::Type type) {
    switch (type) {
        TO_STRING_CASE(Node::Type, Base);
        TO_STRING_CASE(Node::Type, Statement);
        TO_STRING_CASE(Node::Type, Expression);
        TO_STRING_CASE(Node::Type, LetStatement);
        TO_STRING_CASE(Node::Type, ReturnStatement);
        TO_STRING_CASE(Node::Type, ExpressionStatement);
        TO_STRING_CASE(Node::Type, BlockStatement);
        TO_STRING_CASE(Node::Type, Identifier);
        TO_STRING_CASE(Node::Type, Boolean);
        TO_STRING_CASE(Node::Type, IntegerLiteral);
        TO_STRING_CASE(Node::Type, StringLiteral);
        TO_STRING_CASE(Node::Type, PrefixExpression);
        TO_STRING_CASE(Node::Type, InfixExpression);
        TO_STRING_CASE(Node::Type, IfExpression);
        TO_STRING_CASE(Node::Type, FunctionLiteral);
        TO_STRING_CASE(Node::Type, CallExpression);
        TO_STRING_CASE(Node::Type, Program);
        default:
            return "UNKNOWN";
    }
}

std::string LetStatement::toString() const {
    return fmt::format("{} {} = {};", token_.literal, name_->toString(), value_->toString());
}

std::string BlockStatement::toString() const {
    std::stringstream ss;
    ss << "{ ";
    for (const auto& statement : statements_) {
        ss << statement->toString();
    }
    ss << " }";
    return ss.str();
}

std::string FunctionLiteral::toString() const {
    std::string params;
    for (const auto& param : parameters_) {
        params += param->toString() + ", ";
    }
    if (!params.empty()) {
        params.pop_back();
        params.pop_back();
    }
    return fmt::format("{}({}) {}", token_.literal, params, body_->toString());
}

std::string CallExpression::toString() const {
    std::string args;
    for (const auto& arg : arguments_) {
        args += arg->toString() + ", ";
    }
    if (!args.empty()) {
        args.pop_back();
        args.pop_back();
    }
    return fmt::format("{}({})", function_->toString(), args);
}

std::string_view Program::tokenLiteral() const {
    if (statements_.empty()) {
        return "";
    }
    return statements_[0]->tokenLiteral();
}

std::string Program::toString() const {
    std::stringstream ss;
    for (const auto& statement : statements_) {
        ss << statement->toString();
    }
    return ss.str();
}

}  // namespace monkey
}  // namespace pyc