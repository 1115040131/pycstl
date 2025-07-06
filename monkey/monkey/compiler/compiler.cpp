#include "monkey/compiler/compiler.h"

namespace pyc {
namespace monkey {

std::shared_ptr<Error> Compiler::compile(std::shared_ptr<Node> node) {
    switch (node->type()) {
        case Node::Type::Program: {
            auto program = std::dynamic_pointer_cast<Program>(node);
            for (const auto& statement : program->statements()) {
                auto err = compile(statement);
                if (err) {
                    return err;
                }
            }
        } break;
        case Node::Type::ExpressionStatement: {
            auto expression = std::dynamic_pointer_cast<ExpressionStatement>(node);
            auto err = compile(expression->expression());
            if (err) {
                return err;
            }
        } break;
        case Node::Type::IntegerLiteral: {
            auto integer_literal = std::dynamic_pointer_cast<IntegerLiteral>(node);
            auto integer = std::make_shared<Integer>(integer_literal->value());
            auto pos = addConstant(integer);
            emit(OpcodeType::OpConstant, {static_cast<int>(pos)});
        } break;
        case Node::Type::InfixExpression: {
            auto infix = std::dynamic_pointer_cast<InfixExpression>(node);
            auto err = compile(infix->left());
            if (err) {
                return err;
            }
            err = compile(infix->right());
            if (err) {
                return err;
            }
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

size_t Compiler::emit(OpcodeType op, const std::vector<int>& operands) {
    auto instructions = ByteCode::Make(op, operands);
    return addInstruction(instructions);
}

size_t Compiler::addInstruction(const Instructions& instructions) {
    auto pos = instructions_.size();
    instructions_.insert(instructions_.end(), instructions.begin(), instructions.end());
    return pos;
}

}  // namespace monkey
}  // namespace pyc