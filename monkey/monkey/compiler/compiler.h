#pragma once

#include "monkey/code/code.h"
#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

class Compiler {
public:
    static std::shared_ptr<Compiler> New() { return std::make_shared<Compiler>(); }

    std::shared_ptr<Error> compile(std::shared_ptr<Node> node);

    const Instructions& instructions() const { return instructions_; }
    const std::vector<std::shared_ptr<Object>>& constants() const { return constants_; }

private:
    size_t addConstant(std::shared_ptr<Object> object);

    size_t emit(OpcodeType op, const std::vector<int>& operands);

    size_t addInstruction(const Instructions& instructions);

private:
    Instructions instructions_;
    std::vector<std::shared_ptr<Object>> constants_;
};

}  // namespace monkey
}  // namespace pyc