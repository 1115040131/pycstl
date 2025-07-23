#pragma once

#include "monkey/compiler/compiler.h"

namespace pyc {
namespace monkey {

class VM {
public:
    static std::shared_ptr<VM> New(std::shared_ptr<Compiler> compiler) {
        return std::make_shared<VM>(compiler->instructions(), compiler->constants());
    }

    VM(const Instructions& instructions, const std::vector<std::shared_ptr<Object>>& constants)
        : instructions_(instructions), constants_(constants), stack_(kStackSize) {}

public:
    std::shared_ptr<Object> top() const;

    std::shared_ptr<Object> lastPoppedElement() const;

    std::shared_ptr<Object> push(std::shared_ptr<Object> object);

    std::shared_ptr<Object> pop();

    std::shared_ptr<Object> run();

private:
    std::shared_ptr<Object> excuteBinaryOperation(OpcodeType op);

    std::shared_ptr<Object> excuteBinaryIntegerOperation(OpcodeType op, std::shared_ptr<Integer> left,
                                                         std::shared_ptr<Integer> right);

    std::shared_ptr<Object> excuteComparison(OpcodeType op);

    std::shared_ptr<Object> excuteIntegerComparison(OpcodeType op, std::shared_ptr<Integer> left,
                                                    std::shared_ptr<Integer> right);

    std::shared_ptr<Object> excuteBangOperation();

    std::shared_ptr<Object> excuteMinusOperation();

private:
    static constexpr size_t kStackSize = 2048;

    Instructions instructions_;
    std::vector<std::shared_ptr<Object>> constants_;

    std::vector<std::shared_ptr<Object>> stack_;
    size_t sp_{};
};

}  // namespace monkey
}  // namespace pyc