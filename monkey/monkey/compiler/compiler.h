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

struct CompilationScope {
    Instructions instructions_;
    EmittedInstruction last_instruction_;
    EmittedInstruction prev_instruction_;
};

class Compiler {
public:
    static std::shared_ptr<Compiler> New();

    static std::shared_ptr<Compiler> NewWithState(const std::vector<std::shared_ptr<Object>>& constants,
                                                  std::shared_ptr<SymbolTable> symbol_table) {
        auto compiler = New();
        compiler->constants_ = constants;
        compiler->symbol_table_ = symbol_table;
        return compiler;
    }

    std::shared_ptr<Error> compile(std::shared_ptr<Node> node);

    const std::vector<std::shared_ptr<Object>>& constants() const { return constants_; }

    const CompilationScope& scope() const { return scopes_[scope_index_]; }
    const Instructions& instructions() const { return scope().instructions_; }

private:
    size_t addConstant(std::shared_ptr<Object> object);

    size_t emit(OpcodeType op, const std::vector<size_t>& operands);

    void loadSymbol(std::shared_ptr<Symbol> symbol);

    size_t addInstruction(const Instructions& instructions);

    void setLastInstruction(OpcodeType op, size_t position);

    bool isLastInstrction(OpcodeType op) const;

    bool isLastInstructionPop() const;

    void removeLastPop();
    void removeLastPopWithReturn();

    void replaceInstruction(size_t position, const Instructions& new_instructions);

    void changeOperand(size_t position, size_t operand);

    CompilationScope& currentScope() { return scopes_[scope_index_]; }
    Instructions& currentInstructions() { return currentScope().instructions_; }

    void enterScope();
    Instructions leaveScope();

private:
    std::vector<std::shared_ptr<Object>> constants_;
    std::shared_ptr<SymbolTable> symbol_table_ = SymbolTable::New();

    std::vector<CompilationScope> scopes_ = std::vector<CompilationScope>(1);
    size_t scope_index_{};
};

}  // namespace monkey
}  // namespace pyc