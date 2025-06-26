#pragma once

#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

std::shared_ptr<Object> Eval(std::shared_ptr<Node> node, std::shared_ptr<Environment> env);

#pragma region Statement

std::shared_ptr<Object> EvalProgram(std::shared_ptr<Program> program, std::shared_ptr<Environment> env);
std::shared_ptr<Object> EvalBlockStatement(std::shared_ptr<BlockStatement> block,
                                           std::shared_ptr<Environment> env);

#pragma endregion

#pragma region Expression

std::shared_ptr<Object> EvalIdentifier(std::shared_ptr<Identifier> identifier, std::shared_ptr<Environment> env);
std::shared_ptr<Object> EvalPrefixExpression(std::shared_ptr<PrefixExpression> prefix_expression,
                                             std::shared_ptr<Environment> env);
std::shared_ptr<Object> EvalInfixExpression(std::shared_ptr<InfixExpression> infix_expression,
                                            std::shared_ptr<Environment> env);
std::shared_ptr<Object> EvalIfExpression(std::shared_ptr<IfExpression> if_expression,
                                         std::shared_ptr<Environment> env);
std::shared_ptr<Object> EvalFunctionLiteral(std::shared_ptr<FunctionLiteral> function_literal,
                                            std::shared_ptr<Environment> env);
std::shared_ptr<Object> EvalCallExpression(std::shared_ptr<CallExpression> call_expression,
                                           std::shared_ptr<Environment> env);

#pragma endregion

#pragma region Object

std::shared_ptr<BooleanObject> EvalBool(bool value);
std::shared_ptr<Object> EvalBangOperatorExpression(std::shared_ptr<Object> right);
std::shared_ptr<Object> EvalMinusPrefixOperatorExpression(std::shared_ptr<Object> right);

#pragma endregion

#pragma region function

std::vector<std::shared_ptr<Object>> EvalExpressions(const std::vector<std::shared_ptr<Expression>>& expressions,
                                                     std::shared_ptr<Environment> env);
std::shared_ptr<Object> ApplyFunction(std::shared_ptr<Object> object,
                                      const std::vector<std::shared_ptr<Object>>& args);
std::shared_ptr<Environment> ExtendFunctionEnv(std::shared_ptr<Function> function,
                                               const std::vector<std::shared_ptr<Object>>& args);

#pragma endregion

}  // namespace monkey
}  // namespace pyc