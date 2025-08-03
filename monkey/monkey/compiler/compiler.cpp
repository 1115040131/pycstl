#include "monkey/compiler/compiler.h"

#include <algorithm>

#include "monkey/evaluator/evaluator.h"

namespace pyc {
namespace monkey {

std::shared_ptr<Error> Compiler::compile(std::shared_ptr<Node> node) {
    switch (node->type()) {
        case Node::Type::Program: {
            auto program = std::dynamic_pointer_cast<Program>(node);
            for (const auto& statement : program->statements()) {
                if (auto err = compile(statement); IsError(err)) {
                    return err;
                }
            }
        } break;
        case Node::Type::LetStatement: {
            auto let_statement = std::dynamic_pointer_cast<LetStatement>(node);
            if (auto err = compile(let_statement->value()); IsError(err)) {
                return err;
            }
            auto symbol = symbol_table_->Define(let_statement->name()->toString());
            if (symbol->scope == SymbolScopeType::kGlobal) {
                emit(OpcodeType::OpSetGlobal, {symbol->index});
            } else {
                emit(OpcodeType::OpSetLocal, {symbol->index});
            }
        } break;
        case Node::Type::ExpressionStatement: {
            auto expression = std::dynamic_pointer_cast<ExpressionStatement>(node);
            if (auto err = compile(expression->expression()); IsError(err)) {
                return err;
            }
            emit(OpcodeType::OpPop, {});
        } break;
        case Node::Type::BlockStatement: {
            auto block = std::dynamic_pointer_cast<BlockStatement>(node);
            for (const auto& statement : block->statements()) {
                if (auto err = compile(statement); IsError(err)) {
                    return err;
                }
            }
        } break;
        case Node::Type::Identifier: {
            auto identifier = std::dynamic_pointer_cast<Identifier>(node);
            auto symbol = symbol_table_->Resolve(identifier->toString());
            if (!symbol) {
                return std::make_shared<Error>(
                    fmt::format("Undefined identifier: {}", identifier->tokenLiteral()));
            }
            if (symbol->scope == SymbolScopeType::kGlobal) {
                emit(OpcodeType::OpGetGlobal, {symbol->index});
            } else {
                emit(OpcodeType::OpGetLocal, {symbol->index});
            }
        } break;
        case Node::Type::Boolean: {
            auto boolean = std::dynamic_pointer_cast<Boolean>(node);
            if (boolean->value()) {
                emit(OpcodeType::OpTrue, {});
            } else {
                emit(OpcodeType::OpFalse, {});
            }
        } break;
        case Node::Type::IntegerLiteral: {
            auto integer_literal = std::dynamic_pointer_cast<IntegerLiteral>(node);
            auto integer = std::make_shared<Integer>(integer_literal->value());
            auto pos = addConstant(integer);
            emit(OpcodeType::OpConstant, {pos});
        } break;
        case Node::Type::StringLiteral: {
            auto string_literal = std::dynamic_pointer_cast<StringLiteral>(node);
            auto str = std::make_shared<String>(string_literal->toString());
            auto pos = addConstant(str);
            emit(OpcodeType::OpConstant, {pos});
        } break;
        case Node::Type::ArrayLiteral: {
            auto array_literal = std::dynamic_pointer_cast<ArrayLiteral>(node);
            for (const auto& element : array_literal->elements()) {
                if (auto err = compile(element); IsError(err)) {
                    return err;
                }
            }
            emit(OpcodeType::OpArray, {array_literal->elements().size()});
        } break;
        case Node::Type::HashLiteral: {
            auto hash_literal = std::dynamic_pointer_cast<HashLiteral>(node);

#ifdef SORTED_HASH
            std::vector<std::shared_ptr<Expression>> keys;
            for (const auto& [key, _] : hash_literal->pairs()) {
                keys.push_back(key);
            }
            std::ranges::sort(keys,
                              [](const std::shared_ptr<Expression>& lhs, const std::shared_ptr<Expression>& rhs) {
                                  return lhs->toString() < rhs->toString();
                              });
            for (const auto& key : keys) {
                if (auto err = compile(key); IsError(err)) {
                    return err;
                }
                if (auto err = compile(hash_literal->pairs().at(key)); IsError(err)) {
                    return err;
                }
            }
#else
            for (const auto& [key, value] : hash_literal->pairs()) {
                if (auto err = compile(key); IsError(err)) {
                    return err;
                }
                if (auto err = compile(value); IsError(err)) {
                    return err;
                }
            }
#endif
            emit(OpcodeType::OpHash, {hash_literal->pairs().size()});
        } break;
        case Node::Type::IndexExpression: {
            auto index_expression = std::dynamic_pointer_cast<IndexExpression>(node);
            if (auto err = compile(index_expression->left()); IsError(err)) {
                return err;
            }
            if (auto err = compile(index_expression->index()); IsError(err)) {
                return err;
            }
            emit(OpcodeType::OpIndex, {});
        } break;
        case Node::Type::PrefixExpression: {
            auto prefix = std::dynamic_pointer_cast<PrefixExpression>(node);
            if (auto err = compile(prefix->right()); IsError(err)) {
                return err;
            }
            if (prefix->tokenLiteral() == "!") {
                emit(OpcodeType::OpBang, {});
            } else if (prefix->tokenLiteral() == "-") {
                emit(OpcodeType::OpMinus, {});
            } else {
                return std::make_shared<Error>(fmt::format("unknown operator: {}", prefix->tokenLiteral()));
            }
        } break;
        case Node::Type::InfixExpression: {
            auto infix = std::dynamic_pointer_cast<InfixExpression>(node);

            if (infix->tokenLiteral() == "<") {
                if (auto err = compile(infix->right()); IsError(err)) {
                    return err;
                }
                if (auto err = compile(infix->left()); IsError(err)) {
                    return err;
                }
                emit(OpcodeType::OpGreaterThan, {});
                return nullptr;
            }

            if (auto err = compile(infix->left()); IsError(err)) {
                return err;
            }
            if (auto err = compile(infix->right()); IsError(err)) {
                return err;
            }
            if (infix->tokenLiteral() == "+") {
                emit(OpcodeType::OpAdd, {});
            } else if (infix->tokenLiteral() == "-") {
                emit(OpcodeType::OpSub, {});
            } else if (infix->tokenLiteral() == "*") {
                emit(OpcodeType::OpMul, {});
            } else if (infix->tokenLiteral() == "/") {
                emit(OpcodeType::OpDiv, {});
            } else if (infix->tokenLiteral() == ">") {
                emit(OpcodeType::OpGreaterThan, {});
            } else if (infix->tokenLiteral() == "==") {
                emit(OpcodeType::OpEqual, {});
            } else if (infix->tokenLiteral() == "!=") {
                emit(OpcodeType::OpNotEqual, {});
            } else {
                return std::make_shared<Error>(fmt::format("unknown operator: {}", infix->tokenLiteral()));
            }
        } break;
        case Node::Type::IfExpression: {
            auto if_expression = std::dynamic_pointer_cast<IfExpression>(node);

            if (auto err = compile(if_expression->condition()); IsError(err)) {
                return err;
            }

            // 预设一个偏移量方便后续回填
            auto jump_not_true_pos = emit(OpcodeType::OpJumpNotTruthy, {9999});

            if (auto err = compile(if_expression->consequence()); IsError(err)) {
                return err;
            }

            if (isLastInstructionPop()) {
                removeLastPop();
            }

            auto jump_pos = emit(OpcodeType::OpJump, {9999});

            auto after_consequence_pos = currentInstructions().size();
            changeOperand(jump_not_true_pos, after_consequence_pos);

            if (!if_expression->alternative()) {
                emit(OpcodeType::OpNull, {});
            } else {
                if (auto err = compile(if_expression->alternative()); IsError(err)) {
                    return err;
                }

                if (isLastInstructionPop()) {
                    removeLastPop();
                }
            }

            auto after_alternative_pos = currentInstructions().size();
            changeOperand(jump_pos, after_alternative_pos);
        } break;
        case Node::Type::FunctionLiteral: {
            auto function_literal = std::dynamic_pointer_cast<FunctionLiteral>(node);

            // 在新编译作用域编译函数
            enterScope();

            if (auto err = compile(function_literal->body()); IsError(err)) {
                return err;
            }

            // 隐式返回
            if (isLastInstructionPop()) {
                removeLastPopWithReturn();
            }

            // 无返回则强制返回null
            if (!isLastInstrction(OpcodeType::OpReturnValue)) {
                emit(OpcodeType::OpReturn, {});
            }

            // 回到上一层作用域并获取编译字节码
            auto local_num = symbol_table_->nextIndex();
            auto instructions = leaveScope();
            auto pos = addConstant(std::make_shared<CompiledFunction>(instructions, local_num));
            emit(OpcodeType::OpConstant, {pos});
        } break;
        case Node::Type::ReturnStatement: {
            auto return_statement = std::dynamic_pointer_cast<ReturnStatement>(node);

            if (auto err = compile(return_statement->value()); IsError(err)) {
                return err;
            }

            emit(OpcodeType::OpReturnValue, {});
        } break;
        case Node::Type::CallExpression: {
            auto call_expression = std::dynamic_pointer_cast<CallExpression>(node);

            if (auto err = compile(call_expression->function()); IsError(err)) {
                return err;
            }

            emit(OpcodeType::OpCall, {});
        } break;
        default:
            break;
    }
    return nullptr;
}

size_t Compiler::addConstant(std::shared_ptr<Object> object) {
    constants_.push_back(object);
    return constants_.size() - 1;
}

size_t Compiler::emit(OpcodeType op, const std::vector<size_t>& operands) {
    auto instructions = ByteCode::Make(op, operands);
    auto position = addInstruction(instructions);
    setLastInstruction(op, position);
    return position;
}

size_t Compiler::addInstruction(const Instructions& instructions) {
    auto pos = currentInstructions().size();
    currentInstructions().insert(currentInstructions().end(), instructions.begin(), instructions.end());
    return pos;
}

void Compiler::setLastInstruction(OpcodeType op, size_t position) {
    currentScope().prev_instruction_ = currentScope().last_instruction_;
    currentScope().last_instruction_ = {op, position};
}

bool Compiler::isLastInstrction(OpcodeType op) const { return scope().last_instruction_.opcode == op; }

bool Compiler::isLastInstructionPop() const { return isLastInstrction(OpcodeType::OpPop); }

void Compiler::removeLastPop() {
    currentInstructions().resize(currentScope().last_instruction_.position);
    currentScope().last_instruction_ = currentScope().prev_instruction_;
}

void Compiler::removeLastPopWithReturn() {
    replaceInstruction(currentScope().last_instruction_.position, ByteCode::Make(OpcodeType::OpReturnValue, {}));
}

void Compiler::replaceInstruction(size_t position, const Instructions& new_instructions) {
    if (position + new_instructions.size() > currentInstructions().size()) {
        fmt::println("Cannot replace instruction at position {} with size {}, current instructions size: {}",
                     position, new_instructions.size(), currentInstructions().size());
        return;
    }
    std::copy(new_instructions.begin(), new_instructions.end(), currentInstructions().begin() + position);
    setLastInstruction(static_cast<OpcodeType>(currentInstructions()[position]), position);
}

void Compiler::changeOperand(size_t position, size_t operand) {
    auto new_instruction = ByteCode::Make(static_cast<OpcodeType>(currentInstructions()[position]), {operand});
    replaceInstruction(position, new_instruction);
}

void Compiler::enterScope() {
    scopes_.emplace_back();
    scope_index_++;
    symbol_table_ = SymbolTable::NewEnclosed(symbol_table_);
}

Instructions Compiler::leaveScope() {
    if (scope_index_ == 0) {
        throw std::runtime_error("Cannot leave global scope");
    }
    auto instructions = currentInstructions();
    scopes_.pop_back();
    scope_index_--;
    symbol_table_ = symbol_table_->outer();
    return instructions;
}

}  // namespace monkey
}  // namespace pyc