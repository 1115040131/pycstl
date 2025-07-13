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
    stack_[sp_] = nullptr;  // 清除引用以避免悬空指
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
            case OpcodeType::OpAdd:
                auto right = std::dynamic_pointer_cast<Integer>(pop());
                auto left = std::dynamic_pointer_cast<Integer>(pop());
                push(std::make_shared<Integer>(left->value() + right->value()));
                break;
        }

        ip = next_offset;
    }

    return nullptr;
}

}  // namespace monkey
}  // namespace pyc