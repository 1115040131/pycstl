#pragma once

#include "monkey/code/code.h"
#include "monkey/compiler/symbol_table.h"
#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

struct EmittedInstruction {
    OpcodeType opcode;
    size_t position;
};

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

    void setLastInstruction(OpcodeType op, size_t position);

    bool isLastInstructionPop() const;

    void removeLastPop();

    void replaceInstruction(size_t position, const Instructions& new_instructions);

    void changeOperand(size_t position, int operand);

private:
    Instructions instructions_;
    std::vector<std::shared_ptr<Object>> constants_;

    EmittedInstruction last_instruction_;
    EmittedInstruction prev_instruction_;

    std::shared_ptr<SymbolTable> symbol_table_ = SymbolTable::New();
};

}  // namespace monkey
}  // namespace pyc