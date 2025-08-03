#include "monkey/vm/vm.h"

#include <algorithm>

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
    auto frame = currentFrame();
    auto instructions_ = frame->instructions();

    while (frame->ip < instructions_.size()) {
        auto op = static_cast<OpcodeType>(instructions_[frame->ip]);
        auto [operands, next_offset] = ByteCode::ReadOperands(instructions_, frame->ip);

        frame->ip = next_offset;

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
            case OpcodeType::OpDiv: {
                auto result = excuteBinaryOperation(op);
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpTrue: {
                auto result = push(kTrueObj);
                if (IsError(result)) {
                    return result;
                }
            } break;
            case OpcodeType::OpFalse: {
                auto result = push(kFalseObj);
                if (IsError(result)) {
                    return result;
                }
            } break;
            case OpcodeType::OpNull: {
                auto result = push(kNullObj);
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpEqual:
            case OpcodeType::OpNotEqual:
            case OpcodeType::OpGreaterThan: {
                auto result = excuteComparison(op);
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpBang: {
                auto result = excuteBangOperation();
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpMinus: {
                auto result = excuteMinusOperation();
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpJump:
                frame->ip = operands[0];
                break;

            case OpcodeType::OpJumpNotTruthy:
                if (!IsTruthy(pop())) {
                    frame->ip = operands[0];
                }
                break;

            case OpcodeType::OpSetGlobal:
                if (operands[0] >= globals_.size()) {
                    return std::make_shared<Error>("Global index out of range");
                }
                globals_[operands[0]] = pop();
                break;

            case OpcodeType::OpGetGlobal: {
                if (operands[0] >= globals_.size()) {
                    return std::make_shared<Error>("Global index out of range");
                }
                auto global = globals_[operands[0]];
                if (global == nullptr) {
                    return std::make_shared<Error>(
                        fmt::format("undefined global variable at index {}", operands[0]));
                }
                auto result = push(global);
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpSetLocal:
                stack_[frame->bp + operands[0]] = pop();
                break;

            case OpcodeType::OpGetLocal: {
                auto result = push(stack_[frame->bp + operands[0]]);
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpArray: {
                auto array = buildArray(operands[0]);
                if (IsError(array)) {
                    return array;
                }
                auto result = push(array);
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpHash: {
                auto hash = buildHash(operands[0]);
                if (IsError(hash)) {
                    return hash;
                }
                auto result = push(hash);
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpIndex: {
                auto result = executeIndexExpression();
                if (IsError(result)) {
                    return result;
                }
            } break;

            case OpcodeType::OpCall: {
                auto result = callFunction(operands[0]);
                if (IsError(result)) {
                    return result;
                }

                frame = currentFrame();
                instructions_ = frame->instructions();
            } break;
            case OpcodeType::OpReturnValue: {
                auto return_value = pop();

                auto call_frame = popFrame();
                sp_ = call_frame->bp - 1;

                frame = currentFrame();
                instructions_ = frame->instructions();

                // pop();  // 函数本体出栈

                auto result = push(return_value);
                if (IsError(result)) {
                    return result;
                }
            } break;
            case OpcodeType::OpReturn: {
                auto call_frame = popFrame();
                sp_ = call_frame->bp - 1;

                frame = currentFrame();
                instructions_ = frame->instructions();

                // pop();  // 函数本体出栈

                auto result = push(kNullObj);
                if (IsError(result)) {
                    return result;
                }
            } break;

            default:
                break;
        }
    }

    return nullptr;
}

std::shared_ptr<Object> VM::excuteBinaryOperation(OpcodeType op) {
    auto right = pop();
    auto left = pop();

    if (left->type() == Object::Type::INTEGER && right->type() == Object::Type::INTEGER) {
        return excuteBinaryIntegerOperation(op, std::dynamic_pointer_cast<Integer>(left),
                                            std::dynamic_pointer_cast<Integer>(right));
    } else if (left->type() == Object::Type::STRING && right->type() == Object::Type::STRING) {
        return excuteBinaryStringOperation(op, std::dynamic_pointer_cast<String>(left),
                                           std::dynamic_pointer_cast<String>(right));
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
    return std::make_shared<Error>(fmt::format("unknown integer operator: {}", toString(op)));
}

std::shared_ptr<Object> VM::excuteBinaryStringOperation(OpcodeType op, std::shared_ptr<String> left,
                                                        std::shared_ptr<String> right) {
    switch (op) {
        case OpcodeType::OpAdd:
            return push(std::make_shared<String>(left->value() + right->value()));
        default:
            break;
    }
    return std::make_shared<Error>(fmt::format("unknown string operator: {}", toString(op)));
}

std::shared_ptr<Object> VM::excuteComparison(OpcodeType op) {
    auto right = pop();
    auto left = pop();

    if (left->type() == Object::Type::INTEGER && right->type() == Object::Type::INTEGER) {
        return excuteIntegerComparison(op, std::dynamic_pointer_cast<Integer>(left),
                                       std::dynamic_pointer_cast<Integer>(right));
    }
    switch (op) {
        case OpcodeType::OpEqual:
            return push(EvalBool(left == right));
        case OpcodeType::OpNotEqual:
            return push(EvalBool(left != right));
        default:
            break;
    }
    return std::make_shared<Error>(fmt::format("unsupported types for binary operaction: {} {} {}",
                                               left->typeStr(), toString(op), right->typeStr()));
}

std::shared_ptr<Object> VM::excuteIntegerComparison(OpcodeType op, std::shared_ptr<Integer> left,
                                                    std::shared_ptr<Integer> right) {
    switch (op) {
        case OpcodeType::OpGreaterThan:
            return push(EvalBool(left->value() > right->value()));
        case OpcodeType::OpEqual:
            return push(EvalBool(left->value() == right->value()));
        case OpcodeType::OpNotEqual:
            return push(EvalBool(left->value() != right->value()));
        default:
            break;
    }
    return std::make_shared<Error>(fmt::format("unknown integer operator: {}", toString(op)));
}

std::shared_ptr<Object> VM::excuteBangOperation() {
    auto operand = pop();
    if (operand == kTrueObj) {
        return push(kFalseObj);
    } else if (operand == kFalseObj) {
        return push(kTrueObj);
    }
    return push(kFalseObj);
}

std::shared_ptr<Object> VM::excuteMinusOperation() {
    auto operand = pop();
    if (operand->type() != Object::Type::INTEGER) {
        return std::make_shared<Error>(fmt::format("unsupported type for negation: {}", operand->typeStr()));
    }
    auto integer = std::dynamic_pointer_cast<Integer>(operand);
    return push(std::make_shared<Integer>(-integer->value()));
}

std::shared_ptr<Object> VM::executeIndexExpression() {
    auto index = pop();
    auto left = pop();
    if (left->type() == Object::Type::ARRAY && index->type() == Object::Type::INTEGER) {
        auto result =
            EvalArrayIndex(std::dynamic_pointer_cast<Array>(left), std::dynamic_pointer_cast<Integer>(index));
        if (IsError(result)) {
            return result;
        }
        return push(result);
    } else if (left->type() == Object::Type::HASH) {
        auto result = EvalHashIndex(std::dynamic_pointer_cast<Hash>(left), index);
        if (IsError(result)) {
            return result;
        }
        return push(result);
    }
    return std::make_shared<Error>(
        fmt::format("index operator not supported: {}[{}]", left->typeStr(), index->typeStr()));
}

std::shared_ptr<Object> VM::buildArray(size_t size) {
    if (sp_ < size) {
        return std::make_shared<Error>("Stack underflow for array creation");
    }
    std::vector<std::shared_ptr<Object>> elements;
    elements.reserve(size);
    for (size_t i = 0; i < size; i++) {
        elements.push_back(pop());
    }
    std::ranges::reverse(elements);
    return std::make_shared<Array>(std::move(elements));
}

std::shared_ptr<Object> VM::buildHash(size_t size) {
    if (sp_ < size * 2) {
        return std::make_shared<Error>("Stack underflow for hash creation");
    }
    auto hash = std::make_shared<Hash>();
    for (size_t i = 0; i < size; i++) {
        auto value = pop();
        auto key = pop();
        if (!key->hashable()) {
            return std::make_shared<Error>(fmt::format("unhashable type: {}", key->typeStr()));
        }
        hash->pairs()[key->getHashKey()] = {key, value};
    }
    return hash;
}

std::shared_ptr<Object> VM::callFunction(size_t num_args) {
    if (sp_ < 1 + num_args) {
        return std::make_shared<Error>("Stack underflow");
    }

    auto function_obj = stack_[sp_ - 1 - num_args];
    if (function_obj->type() != Object::Type::COMPILED_FUNCTION) {
        return std::make_shared<Error>(fmt::format("calling non-function: {}", function_obj->typeStr()));
    }

    auto compiled_function = std::dynamic_pointer_cast<CompiledFunction>(function_obj);
    if (compiled_function->parametersNum() != num_args) {
        return std::make_shared<Error>(fmt::format("wrong number of arguments: want={}, got={}",
                                                   compiled_function->parametersNum(), num_args));
    }

    auto function_frame = Frame::New(compiled_function, sp_ - num_args);

    pushFrame(function_frame);

    // 借用一段调用栈空间
    sp_ = function_frame->bp + compiled_function->localNum();

    return nullptr;
}

void VM::pushFrame(std::shared_ptr<Frame> frame) {
    if (frame_index_ >= kFrameSize) {
        throw std::runtime_error("Frame overflow");
    }
    frame_index_++;
    frames_[frame_index_] = frame;
}

std::shared_ptr<Frame> VM::popFrame() {
    if (frame_index_ == 0) {
        throw std::runtime_error("No frames to pop");
    }
    auto frame = frames_[frame_index_];
    frame_index_--;
    return frame;
}

}  // namespace monkey
}  // namespace pyc