#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace pyc {
namespace monkey {

using SymbolScope = std::string;

namespace SymbolScopeType {
inline constexpr SymbolScope kGlobal = "GLOBAL";
inline constexpr SymbolScope kLocal = "LOCAL";
inline constexpr SymbolScope kBuiltin = "BUILTIN";
inline constexpr SymbolScope kFree = "FREE";
inline constexpr SymbolScope kFunctionScope = "FUNCTION";
}  // namespace SymbolScopeType

struct Symbol {
    std::string name;
    SymbolScope scope;
    size_t index;

    inline constexpr auto operator<=>(const Symbol&) const = default;
};

class SymbolTable {
public:
    static std::shared_ptr<SymbolTable> New() { return std::make_shared<SymbolTable>(); }

    static std::shared_ptr<SymbolTable> NewEnclosed(std::shared_ptr<SymbolTable> outer) {
        auto symbol_table = std::make_shared<SymbolTable>();
        symbol_table->outer_ = std::move(outer);
        return symbol_table;
    }

    std::shared_ptr<Symbol> Define(const std::string& name);

    std::shared_ptr<Symbol> DefineBuiltin(const std::string& name, size_t index);

    std::shared_ptr<Symbol> DefineFree(std::shared_ptr<Symbol> original);

    std::shared_ptr<Symbol> DefineFunctionName(const std::string& name);

    std::shared_ptr<Symbol> Resolve(const std::string& name);

    std::shared_ptr<SymbolTable> outer() const { return outer_; }
    size_t nextIndex() const { return next_index_; }

    const std::vector<std::shared_ptr<Symbol>>& freeSymbols() const { return free_symbols_; }

private:
    std::shared_ptr<SymbolTable> outer_;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> store_;
    size_t next_index_ = 0;
    std::vector<std::shared_ptr<Symbol>> free_symbols_;
};

}  // namespace monkey
}  // namespace pyc