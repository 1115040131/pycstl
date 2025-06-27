#pragma once

#include <memory>
#include <string>

#include "monkey/ast/ast.h"
#include "monkey/macro.h"

namespace pyc {
namespace monkey {

class Object {
public:
    enum class Type { Null, ERROR, INTEGER, BOOLEAN, STRING, RETURN_VALUE, FUNCTION, ARRAY, BUILTIN };

    virtual ~Object() = default;
    virtual Type type() const { return Type::Null; }
    virtual std::string inspect() const { return ""; }

    std::string_view typeStr() const;
};

std::string_view toString(Object::Type type);

class Null : public Object {
public:
    TYPE(Null)

    Null() = default;
    virtual ~Null() override = default;

    virtual std::string inspect() const override { return "null"; }
};

class Error : public Object {
public:
    TYPE(ERROR)

    Error(std::string message) : message_(std::move(message)) {}
    virtual ~Error() override = default;

    virtual std::string inspect() const override { return message_; }

private:
    std::string message_;
};

class Integer : public Object {
public:
    TYPE(INTEGER)

    Integer(long long value) : value_(value) {}
    virtual ~Integer() override = default;

    virtual std::string inspect() const override { return std::to_string(value_); }

    long long value() const { return value_; }

private:
    long long value_;
};

class BooleanObject : public Object {
public:
    TYPE(BOOLEAN)

    BooleanObject(bool value) : value_(value) {}
    virtual ~BooleanObject() override = default;

    virtual std::string inspect() const override { return value_ ? "true" : "false"; }

    bool value() const { return value_; }

private:
    bool value_;
};

class String : public Object {
public:
    TYPE(STRING)

    String(std::string value) : value_(std::move(value)) {}
    virtual ~String() override = default;

    virtual std::string inspect() const override { return fmt::format("\"{}\"", value_); }

    const std::string& value() const { return value_; }

private:
    std::string value_;
};

class ReturnValue : public Object {
public:
    TYPE(RETURN_VALUE)

    ReturnValue(std::shared_ptr<Object> value) : value_(std::move(value)) {}
    virtual ~ReturnValue() override = default;

    virtual std::string inspect() const override { return value_->inspect(); }

    const std::shared_ptr<Object>& value() const { return value_; }

private:
    std::shared_ptr<Object> value_;
};

class Environment;

class Function : public Object {
public:
    TYPE(FUNCTION)

    Function() = default;

    virtual ~Function() override = default;

    virtual std::string inspect() const override;

    void setParameters(std::vector<std::shared_ptr<Identifier>> parameters) {
        parameters_ = std::move(parameters);
    }
    const std::vector<std::shared_ptr<Identifier>>& parameters() const { return parameters_; }

    void setBody(std::shared_ptr<BlockStatement> body) { body_ = std::move(body); }
    const std::shared_ptr<BlockStatement>& body() const { return body_; }

    void setEnv(std::shared_ptr<Environment> env) { env_ = std::move(env); }
    const std::shared_ptr<Environment>& env() const { return env_; }

private:
    std::vector<std::shared_ptr<Identifier>> parameters_;
    std::shared_ptr<BlockStatement> body_;
    std::shared_ptr<Environment> env_;
};

class Array : public Object {
public:
    TYPE(ARRAY)

    Array(std::vector<std::shared_ptr<Object>> elements) : elements_(std::move(elements)) {}

    virtual ~Array() override = default;

    virtual std::string inspect() const override;

    std::vector<std::shared_ptr<Object>>& elements() { return elements_; }

private:
    std::vector<std::shared_ptr<Object>> elements_;
};

class Builtin : public Object {
public:
    using BuiltinFunction = std::shared_ptr<Object> (*)(const std::vector<std::shared_ptr<Object>>&);

    TYPE(BUILTIN)

    Builtin(BuiltinFunction function) : function_(std::move(function)) {}

    virtual ~Builtin() override = default;

    virtual std::string inspect() const override { return "builtin function"; }

    const BuiltinFunction& function() const { return function_; }

private:
    BuiltinFunction function_;
};

inline auto kNullObj = std::make_shared<Null>();
inline auto kTrueObj = std::make_shared<BooleanObject>(true);
inline auto kFalseObj = std::make_shared<BooleanObject>(false);

}  // namespace monkey
}  // namespace pyc