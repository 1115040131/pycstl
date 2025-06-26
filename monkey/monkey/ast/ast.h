#pragma once

#include <memory>
#include <string>
#include <vector>

#include "monkey/macro.h"
#include "monkey/token/token.h"

namespace pyc {
namespace monkey {

class Node {
public:
    enum class Type {
        Base,

        Statement,   // 语句类型
        Expression,  // 表达式类型

        LetStatement,         // Let语句
        ReturnStatement,      // Return语句
        ExpressionStatement,  // 表达式语句
        BlockStatement,       // 代码块

        Identifier,        // 标志符
        Boolean,           // 布尔
        IntegerLiteral,    // 整数
        StringLiteral,     // 字符串
        PrefixExpression,  // 前缀表达式
        InfixExpression,   // 中缀表达式
        IfExpression,      // If表达式
        FunctionLiteral,   // 函数定义
        CallExpression,    // 调用

        Program,  // 程序
    };

    virtual ~Node() = default;

    virtual bool good() const { return true; }
    virtual Type type() const { return Type::Base; }

    virtual std::string_view tokenLiteral() const { return ""; }
    virtual std::string toString() const { return ""; }
};

std::string_view toString(Node::Type type);

class Statement : public Node {
public:
    TYPE(Statement)

    virtual ~Statement() override = default;
};

struct Expression : public Node {
public:
    TYPE(Expression)

    virtual ~Expression() override = default;
};

#pragma region Statement

class Identifier;

class LetStatement : public Statement {
public:
    TYPE(LetStatement)

    LetStatement(Token token) : token_(token) {}
    virtual ~LetStatement() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override;

    void setName(std::unique_ptr<Identifier> name) { name_ = std::move(name); }
    const std::unique_ptr<Identifier>& name() const { return name_; }
    void setValue(std::unique_ptr<Expression> value) { value_ = std::move(value); }
    const std::shared_ptr<Expression>& value() const { return value_; }

private:
    Token token_;  // let 关键字
    std::unique_ptr<Identifier> name_;
    std::shared_ptr<Expression> value_;
};

class ReturnStatement : public Statement {
public:
    TYPE(ReturnStatement)

    ReturnStatement(Token token) : token_(token) {}
    virtual ~ReturnStatement() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override {
        return fmt::format("{} {};", token_.literal, value_->toString());
    }

    void setValue(std::unique_ptr<Expression> value) { value_ = std::move(value); }
    const std::shared_ptr<Expression>& value() const { return value_; }

private:
    Token token_;  // return  关键字
    std::shared_ptr<Expression> value_;
};

class ExpressionStatement : public Statement {
public:
    TYPE(ExpressionStatement)

    ExpressionStatement(Token token) : token_(token) {}
    virtual ~ExpressionStatement() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override {
        if (expression_->good()) {
            return expression_->toString();
        }
        return "";
    }

    void setExpression(std::unique_ptr<Expression> expression) { expression_ = std::move(expression); }
    const std::shared_ptr<Expression>& expression() const { return expression_; }

private:
    Token token_;  // 表达式第一个 token
    std::shared_ptr<Expression> expression_;
};

class BlockStatement : public Statement {
public:
    TYPE(BlockStatement)

    BlockStatement(Token token) : token_(token) {}
    virtual ~BlockStatement() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override;

    void addStatement(std::unique_ptr<Statement> statement) { statements_.emplace_back(std::move(statement)); }
    const std::vector<std::shared_ptr<Statement>>& statements() const { return statements_; }

private:
    Token token_;  // {
    std::vector<std::shared_ptr<Statement>> statements_;
};

#pragma endregion

#pragma region Expression

class Identifier : public Expression {
public:
    TYPE(Identifier)

    Identifier(Token token) : token_(token) {}
    virtual ~Identifier() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override { return std::string(token_.literal); }

private:
    Token token_;
};

class Boolean : public Expression {
public:
    TYPE(Boolean)

    Boolean(Token token, bool value) : token_(token), value_(value) {}
    virtual ~Boolean() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override { return std::string(token_.literal); }

    bool value() const { return value_; }

private:
    Token token_;  // true or false
    bool value_;
};

class IntegerLiteral : public Expression {
public:
    TYPE(IntegerLiteral)

    IntegerLiteral(Token token) : token_(token) {}
    virtual ~IntegerLiteral() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override { return std::string(token_.literal); }

    void setValue(long long value) { value_ = value; }
    long long value() const { return value_; }

private:
    Token token_;
    long long value_;
};

class StringLiteral : public Expression {
public:
    TYPE(StringLiteral)

    StringLiteral(Token token) : token_(token) {}
    virtual ~StringLiteral() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override { return std::string(token_.literal); }

private:
    Token token_;
};

class PrefixExpression : public Expression {
public:
    TYPE(PrefixExpression)

    PrefixExpression(Token token) : token_(token) {}
    virtual ~PrefixExpression() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override {
        return fmt::format("({}{})", token_.literal, right_ ? right_->toString() : "");
    }

    void setRight(std::unique_ptr<Expression> right) { right_ = std::move(right); }
    const std::shared_ptr<Expression>& right() const { return right_; }

private:
    Token token_;
    std::shared_ptr<Expression> right_;
};

class InfixExpression : public Expression {
public:
    TYPE(InfixExpression)

    InfixExpression(Token token, std::unique_ptr<Expression> left) : token_(token), left_(std::move(left)) {}
    virtual ~InfixExpression() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override {
        return fmt::format("({} {} {})", left_ ? left_->toString() : "", token_.literal,
                           right_ ? right_->toString() : "");
    }

    const std::shared_ptr<Expression>& left() const { return left_; }

    void setRight(std::unique_ptr<Expression> right) { right_ = std::move(right); }
    const std::shared_ptr<Expression>& right() const { return right_; }

private:
    Token token_;
    std::shared_ptr<Expression> left_;
    std::shared_ptr<Expression> right_;
};

class IfExpression : public Expression {
public:
    TYPE(IfExpression)

    IfExpression(Token token) : token_(token) {}
    virtual ~IfExpression() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override {
        return fmt::format("{} {} {}{}", token_.literal, condition_->toString(),
                           consequence_ ? consequence_->toString() : "",
                           alternative_ ? fmt::format(" else {}", alternative_->toString()) : "");
    }

    void setCondition(std::unique_ptr<Expression> condition) { condition_ = std::move(condition); }
    const std::shared_ptr<Expression>& condition() const { return condition_; }

    void setConsequence(std::unique_ptr<BlockStatement> consequence) { consequence_ = std::move(consequence); }
    const std::shared_ptr<BlockStatement>& consequence() const { return consequence_; }

    void setAlternative(std::unique_ptr<BlockStatement> alternative) { alternative_ = std::move(alternative); }
    const std::shared_ptr<BlockStatement>& alternative() const { return alternative_; }

private:
    Token token_;
    std::shared_ptr<Expression> condition_;
    std::shared_ptr<BlockStatement> consequence_;
    std::shared_ptr<BlockStatement> alternative_;
};

class FunctionLiteral : public Expression {
public:
    TYPE(FunctionLiteral)

    FunctionLiteral(Token token) : token_(token) {}
    virtual ~FunctionLiteral() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override;

    void setParameters(std::vector<std::shared_ptr<Identifier>> parameters) {
        parameters_ = std::move(parameters);
    }
    const std::vector<std::shared_ptr<Identifier>>& parameters() const { return parameters_; }

    void setBody(std::shared_ptr<BlockStatement> body) { body_ = std::move(body); }
    const std::shared_ptr<BlockStatement>& body() const { return body_; }

private:
    Token token_;  // fn 关键字
    std::vector<std::shared_ptr<Identifier>> parameters_;
    std::shared_ptr<BlockStatement> body_;
};

class CallExpression : public Expression {
public:
    TYPE(CallExpression)

    CallExpression(Token token, std::unique_ptr<Expression> function)
        : token_(token), function_(std::move(function)) {}
    virtual ~CallExpression() override = default;

    virtual std::string_view tokenLiteral() const override { return token_.literal; }
    virtual std::string toString() const override;

    const std::shared_ptr<Expression>& function() const { return function_; }

    void setArguments(std::vector<std::shared_ptr<Expression>> argument) { arguments_ = std::move(argument); }
    const std::vector<std::shared_ptr<Expression>>& arguments() const { return arguments_; }

private:
    Token token_;  // ( 关键字
    std::shared_ptr<Expression> function_;
    std::vector<std::shared_ptr<Expression>> arguments_;
};

#pragma endregion

#pragma region Program

// ast 根节点
class Program : public Node {
public:
    virtual std::string_view tokenLiteral() const override;
    virtual std::string toString() const override;

    virtual Type type() const override { return Type::Program; }

    void addStatement(std::unique_ptr<Statement> statement) { statements_.emplace_back(std::move(statement)); }
    const std::vector<std::shared_ptr<Statement>>& statements() const { return statements_; }

private:
    std::vector<std::shared_ptr<Statement>> statements_;
};

#pragma endregion

}  // namespace monkey
}  // namespace pyc
