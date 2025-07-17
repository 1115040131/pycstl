#include "monkey/vm/vm.h"

#include "monkey/evaluator/evaluator.h"

namespace pyc {
namespace monkey {

std::shared_ptr<Object> VM::top() const {
    if (sp_ == 0) {
        return nullptr;
    }
    return stack_[sp_ - 1];
}

std::shared_ptr<Object> VM::lastPoppedElement() const { return stack_[sp_]; }

std::shared_ptr<Object> VM::push(std::shared_ptr<Object> object) {
    if (sp_ >= kStackSize) {
        return std::make_shared<Error>("Stack overflow");
    }

    stack_[sp_] = object;
    sp_++;

    return object;
}

std::shared_ptr<Object> VM::pop() {
    auto object = top();
    if (object == nullptr) {
        return std::make_shared<Error>("Stack underflow");
    }
    sp_--;
    return object;
}

std::shared_ptr<Object> VM::run() {
    size_t ip = 0;
    while (ip < instructions_.size()) {
        auto op = static_cast<OpcodeType>(instructions_[ip]);
        auto [operands, next_offset] = ByteCode::ReadOperands(instructions_, ip);

        switch (op) {
            case OpcodeType::OpConstant: {
                auto result = push(constants_[operands[0]]);
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpPop:
                pop();
                break;

            case OpcodeType::OpAdd:
            case OpcodeType::OpSub:
            case OpcodeType::OpMul:
            case OpcodeType::OpDiv:
                excuteBinaryOperation(op);
                break;

            default:
                break;
        }

        ip = next_offset;
    }

    return nullptr;
}

std::shared_ptr<Object> VM::excuteBinaryOperation(OpcodeType op) {
    auto right = pop();
    auto left = pop();

    if (left->type() == Object::Type::INTEGER && right->type() == Object::Type::INTEGER) {
        excuteBinaryIntegerOperation(op, std::dynamic_pointer_cast<Integer>(left),
                                     std::dynamic_pointer_cast<Integer>(right));
    }
    return std::make_shared<Error>(fmt::format("unsupported types for binary operaction: {} {} {}",
                                               left->typeStr(), toString(op), right->typeStr()));
}

std::shared_ptr<Object> VM::excuteBinaryIntegerOperation(OpcodeType op, std::shared_ptr<Integer> left,
                                                         std::shared_ptr<Integer> right) {
    switch (op) {
        case OpcodeType::OpAdd:
            return push(std::make_shared<Integer>(left->value() + right->value()));
        case OpcodeType::OpSub:
            return push(std::make_shared<Integer>(left->value() - right->value()));
        case OpcodeType::OpMul:
            return push(std::make_shared<Integer>(left->value() * right->value()));
        case OpcodeType::OpDiv:
            if (right->value() == 0) {
                return std::make_shared<Error>("Division by zero");
            }
            return push(std::make_shared<Integer>(left->value() / right->value()));
        default:
            break;
    }
    return std::make_shared<Error>(fmt::format("unknow integer operator: {}", toString(op)));
}

}  // namespace monkey
}  // namespace pyc