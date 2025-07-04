#include "monkey/evaluator/evaluator.h"

#include "monkey/evaluator/builtins.h"
#include "monkey/object/environment.h"

namespace pyc {
namespace monkey {

bool IsError(const std::shared_ptr<Object>& obj) { return obj && obj->type() == Object::Type::ERROR; }

bool IsTruthy(const std::shared_ptr<Object>& obj) {
    if (!obj) {
        return false;
    }
    if (obj->type() == Object::Type::BOOLEAN) {
        return std::dynamic_pointer_cast<BooleanObject>(obj)->value();
    } else if (obj->type() == Object::Type::INTEGER) {
        return std::dynamic_pointer_cast<Integer>(obj)->value() != 0;
    } else if (obj->type() == Object::Type::Null) {
        return false;
    }
    return true;
}

std::shared_ptr<Object> Eval(std::shared_ptr<Node> node, std::shared_ptr<Environment> env) {
    if (!node) {
        return nullptr;
    }
    switch (node->type()) {
        case Node::Type::Program:
            return EvalProgram(std::dynamic_pointer_cast<Program>(node), env);

        // Statements
        case Node::Type::LetStatement: {
            auto let_statement = std::dynamic_pointer_cast<LetStatement>(node);
            auto value = Eval(let_statement->value(), env);
            if (IsError(value)) {
                return value;
            }
            env->set(let_statement->name()->tokenLiteral(), value);
            break;
        }
        case Node::Type::ReturnStatement: {
            auto value = Eval(std::dynamic_pointer_cast<ReturnStatement>(node)->value(), env);
            if (IsError(value)) {
                return value;
            }
            return std::make_shared<ReturnValue>(value);
        }
        case Node::Type::ExpressionStatement:
            return Eval(std::dynamic_pointer_cast<ExpressionStatement>(node)->expression(), env);
        case Node::Type::BlockStatement:
            return EvalBlockStatement(std::dynamic_pointer_cast<BlockStatement>(node), env);

        // Expressions
        case Node::Type::Identifier:
            return EvalIdentifier(std::dynamic_pointer_cast<Identifier>(node), env);
        case Node::Type::Boolean:
            return EvalBool(std::dynamic_pointer_cast<Boolean>(node)->value());
        case Node::Type::IntegerLiteral:
            return std::make_shared<Integer>(std::dynamic_pointer_cast<IntegerLiteral>(node)->value());
        case Node::Type::StringLiteral:
            return std::make_shared<String>(std::dynamic_pointer_cast<StringLiteral>(node)->toString());
        case Node::Type::ArrayLiteral:
            return EvalArrayLiteral(std::dynamic_pointer_cast<ArrayLiteral>(node), env);
        case Node::Type::HashLiteral:
            return EvalHashLiteral(std::dynamic_pointer_cast<HashLiteral>(node), env);
        case Node::Type::IndexExpression:
            return EvalIndexExpression(std::dynamic_pointer_cast<IndexExpression>(node), env);
        case Node::Type::PrefixExpression:
            return EvalPrefixExpression(std::dynamic_pointer_cast<PrefixExpression>(node), env);
        case Node::Type::InfixExpression:
            return EvalInfixExpression(std::dynamic_pointer_cast<InfixExpression>(node), env);
        case Node::Type::IfExpression:
            return EvalIfExpression(std::dynamic_pointer_cast<IfExpression>(node), env);
        case Node::Type::FunctionLiteral:
            return EvalFunctionLiteral(std::dynamic_pointer_cast<FunctionLiteral>(node), env);
        case Node::Type::CallExpression:
            return EvalCallExpression(std::dynamic_pointer_cast<CallExpression>(node), env);

        default:
            break;
    }
    return nullptr;
}

#pragma region Statement

// 对AST节点遍历执行，执行过程中遇到ERROR或Return就中止遍历且返回
std::shared_ptr<Object> EvalProgram(std::shared_ptr<Program> program, std::shared_ptr<Environment> env) {
    auto result = std::make_shared<Object>();
    for (const auto& statement : program->statements()) {
        result = Eval(statement, env);
        if (result) {
            if (result->type() == Object::Type::RETURN_VALUE) {
                return std::dynamic_pointer_cast<ReturnValue>(result)->value();
            } else if (result->type() == Object::Type::ERROR) {
                return result;
            }
        }
    }
    return result;
}

std::shared_ptr<Object> EvalBlockStatement(std::shared_ptr<BlockStatement> block,
                                           std::shared_ptr<Environment> env) {
    auto result = std::make_shared<Object>();
    for (const auto& statement : block->statements()) {
        result = Eval(statement, env);
        if (result && (result->type() == Object::Type::RETURN_VALUE || result->type() == Object::Type::ERROR)) {
            return result;
        }
    }
    return result;
}

#pragma endregion

#pragma region Expression

std::shared_ptr<Object> EvalIdentifier(std::shared_ptr<Identifier> identifier, std::shared_ptr<Environment> env) {
    if (auto value = env->get(identifier->tokenLiteral())) {
        return value;
    } else if (auto fit = GetBuiltin(identifier->tokenLiteral())) {
        return fit;
    }
    return std::make_shared<Error>(fmt::format("identifier not found: {}", identifier->tokenLiteral()));
}

std::shared_ptr<Object> EvalArrayLiteral(std::shared_ptr<ArrayLiteral> array_literal,
                                         std::shared_ptr<Environment> env) {
    auto elements = EvalExpressions(array_literal->elements(), env);
    if (elements.size() == 1 && IsError(elements[0])) {
        return elements[0];
    }
    return std::make_shared<Array>(std::move(elements));
}

std::shared_ptr<Object> EvalHashLiteral(std::shared_ptr<HashLiteral> hash_literal,
                                        std::shared_ptr<Environment> env) {
    auto hash = std::make_shared<Hash>();
    for (const auto& [key_expression, value_expression] : hash_literal->pairs()) {
        auto key = Eval(key_expression, env);
        if (IsError(key)) {
            return key;
        }
        if (!key->hashable()) {
            return std::make_shared<Error>(fmt::format("unusable as hash key: {}", key->typeStr()));
        }
        auto value = Eval(value_expression, env);
        if (IsError(value)) {
            return value;
        }
        hash->pairs()[key->getHashKey()] = {key, value};
    }
    return hash;
}

std::shared_ptr<Object> EvalIndexExpression(std::shared_ptr<IndexExpression> index_expression,
                                            std::shared_ptr<Environment> env) {
    auto left = Eval(index_expression->left(), env);
    if (IsError(left)) {
        return left;
    }
    auto index = Eval(index_expression->index(), env);
    if (IsError(index)) {
        return index;
    }
    if (left->type() == Object::Type::ARRAY && index->type() == Object::Type::INTEGER) {
        return EvalArrayIndex(std::dynamic_pointer_cast<Array>(left), std::dynamic_pointer_cast<Integer>(index));
    } else if (left->type() == Object::Type::HASH) {
        return EvalHashIndex(std::dynamic_pointer_cast<Hash>(left), index);
    }
    return std::make_shared<Error>(
        fmt::format("index operator not supported: {}[{}]", left->typeStr(), index->typeStr()));
}

std::shared_ptr<Object> EvalPrefixExpression(std::shared_ptr<PrefixExpression> prefix_expression,
                                             std::shared_ptr<Environment> env) {
    auto right = Eval(prefix_expression->right(), env);
    if (IsError(right)) {
        return right;
    }
    if (prefix_expression->tokenLiteral() == "!") {
        return EvalBangOperatorExpression(right);
    } else if (prefix_expression->tokenLiteral() == "-") {
        return EvalMinusPrefixOperatorExpression(right);
    }
    return std::make_shared<Error>(fmt::format("unknown operator: {} {}", prefix_expression->tokenLiteral(),
                                               prefix_expression->right()->toString()));
}

std::shared_ptr<Object> EvalInfixExpression(std::shared_ptr<InfixExpression> infix_expression,
                                            std::shared_ptr<Environment> env) {
    auto left = Eval(infix_expression->left(), env);
    if (IsError(left)) {
        return left;
    }
    auto right = Eval(infix_expression->right(), env);
    if (IsError(right)) {
        return right;
    }
    auto operator_str = infix_expression->tokenLiteral();
    if (left->type() != right->type()) {
        return std::make_shared<Error>(
            fmt::format("type mismatch: {} {} {}", left->typeStr(), operator_str, right->typeStr()));
    }
    switch (left->type()) {
        case Object::Type::INTEGER: {
            auto left_value = std::dynamic_pointer_cast<Integer>(left)->value();
            auto right_value = std::dynamic_pointer_cast<Integer>(right)->value();
            if (operator_str == "+") {
                return std::make_shared<Integer>(left_value + right_value);
            } else if (operator_str == "-") {
                return std::make_shared<Integer>(left_value - right_value);
            } else if (operator_str == "*") {
                return std::make_shared<Integer>(left_value * right_value);
            } else if (operator_str == "/") {
                return std::make_shared<Integer>(left_value / right_value);
            } else if (operator_str == "<") {
                return EvalBool(left_value < right_value);
            } else if (operator_str == ">") {
                return EvalBool(left_value > right_value);
            } else if (operator_str == "==") {
                return EvalBool(left_value == right_value);
            } else if (operator_str == "!=") {
                return EvalBool(left_value != right_value);
            }
        } break;
        case Object::Type::BOOLEAN:
            if (operator_str == "==") {
                return EvalBool(std::dynamic_pointer_cast<BooleanObject>(left)->value() ==
                                std::dynamic_pointer_cast<BooleanObject>(right)->value());
            } else if (operator_str == "!=") {
                return EvalBool(std::dynamic_pointer_cast<BooleanObject>(left)->value() !=
                                std::dynamic_pointer_cast<BooleanObject>(right)->value());
            }
            break;
        case Object::Type::STRING:
            if (operator_str != "+") {
                return std::make_shared<Error>(
                    fmt::format("unknown operator: {} {} {}", left->typeStr(), operator_str, right->typeStr()));
            }
            return std::make_shared<String>(fmt::format("{}{}", std::dynamic_pointer_cast<String>(left)->value(),
                                                        std::dynamic_pointer_cast<String>(right)->value()));
        default:
            break;
    }
    return std::make_shared<Error>(
        fmt::format("unknown operator: {} {} {}", left->typeStr(), operator_str, right->typeStr()));
}

std::shared_ptr<Object> EvalIfExpression(std::shared_ptr<IfExpression> if_expression,
                                         std::shared_ptr<Environment> env) {
    auto condition = Eval(if_expression->condition(), env);
    if (IsError(condition)) {
        return condition;
    }
    if (IsTruthy(condition)) {
        return Eval(if_expression->consequence(), env);
    } else if (if_expression->alternative()) {
        return Eval(if_expression->alternative(), env);
    }
    return kNullObj;
}

std::shared_ptr<Object> EvalFunctionLiteral(std::shared_ptr<FunctionLiteral> function_literal,
                                            std::shared_ptr<Environment> env) {
    auto function = std::make_shared<Function>();
    function->setParameters(function_literal->parameters());
    function->setBody(function_literal->body());
    function->setEnv(env);
    return function;
}

std::shared_ptr<Object> EvalCallExpression(std::shared_ptr<CallExpression> call_expression,
                                           std::shared_ptr<Environment> env) {
    auto function = Eval(call_expression->function(), env);
    if (IsError(function)) {
        return function;
    }
    auto args = EvalExpressions(call_expression->arguments(), env);
    if (args.size() == 1 && IsError(args[0])) {
        return args[0];
    }
    return ApplyFunction(function, args);
}

#pragma endregion

#pragma region Object

std::shared_ptr<BooleanObject> EvalBool(bool value) {
    if (value) {
        return kTrueObj;
    }
    return kFalseObj;
}

std::shared_ptr<Object> EvalBangOperatorExpression(std::shared_ptr<Object> right) {
    if (IsTruthy(right)) {
        return kFalseObj;
    }
    return kTrueObj;
}

std::shared_ptr<Object> EvalMinusPrefixOperatorExpression(std::shared_ptr<Object> right) {
    if (right->type() != Object::Type::INTEGER) {
        return std::make_shared<Error>(fmt::format("unknown operator: -{}", toString(right->type())));
    }
    return std::make_shared<Integer>(-std::dynamic_pointer_cast<Integer>(right)->value());
}

#pragma endregion

#pragma region Index

std::shared_ptr<Object> EvalArrayIndex(std::shared_ptr<Array> array, std::shared_ptr<Integer> index) {
    if (index->value() < 0 || index->value() >= static_cast<long long>(array->elements().size())) {
        return std::make_shared<Error>(
            fmt::format("index {} out of bounds: {}", index->value(), array->elements().size()));
    }
    return array->elements()[index->value()];
}

std::shared_ptr<Object> EvalHashIndex(std::shared_ptr<Hash> hash, std::shared_ptr<Object> index) {
    if (!index->hashable()) {
        return std::make_shared<Error>(fmt::format("unusable as hash key: {}", index->typeStr()));
    }
    auto iter = hash->pairs().find(index->getHashKey());
    if (iter == hash->pairs().end()) {
        return kNullObj;  // Return null if the key is not found
    }
    return iter->second.value;  // Return the value associated with the key
}

#pragma endregion

#pragma region function

std::vector<std::shared_ptr<Object>> EvalExpressions(const std::vector<std::shared_ptr<Expression>>& expressions,
                                                     std::shared_ptr<Environment> env) {
    std::vector<std::shared_ptr<Object>> result;
    for (const auto& expression : expressions) {
        auto evaluated = Eval(expression, env);
        if (IsError(evaluated)) {
            return {evaluated};
        }
        result.emplace_back(evaluated);
    }
    return result;
}

std::shared_ptr<Object> ApplyFunction(std::shared_ptr<Object> object,
                                      const std::vector<std::shared_ptr<Object>>& args) {
    if (auto function = std::dynamic_pointer_cast<Function>(object)) {
        auto extended_env = ExtendFunctionEnv(function, args);
        auto evaluated = Eval(function->body(), extended_env);
        auto return_value = std::dynamic_pointer_cast<ReturnValue>(evaluated);
        if (return_value) {
            return return_value->value();
        }
        return evaluated;
    } else if (auto builtin = std::dynamic_pointer_cast<Builtin>(object)) {
        return builtin->function()(args);
    }
    return std::make_shared<Error>(fmt::format("not a function: {}", object->typeStr()));
}

std::shared_ptr<Environment> ExtendFunctionEnv(std::shared_ptr<Function> function,
                                               const std::vector<std::shared_ptr<Object>>& args) {
    auto env = Environment::NewEnclosed(function->env());
    for (size_t i = 0; i < function->parameters().size(); ++i) {
        auto param = function->parameters()[i];
        if (i < args.size()) {
            env->set(param->tokenLiteral(), args[i]);
        }
    }
    return env;
}

#pragma endregion

}  // namespace monkey
}  // namespace pyc