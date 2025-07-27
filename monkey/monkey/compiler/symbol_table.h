#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace pyc {
namespace monkey {

using SymbolScope = std::string;

namespace SymbolScopeType {
inline constexpr SymbolScope kGlobal = "GLOBAL";
}

struct Symbol {
    std::string name;
    SymbolScope scope;
    size_t index;

    inline constexpr auto operator<=>(const Symbol&) const = default;
};

class SymbolTable {
public:
    static std::shared_ptr<SymbolTable> New() { return std::make_shared<SymbolTable>(); }

    std::shared_ptr<Symbol> Define(const std::string& name);

    std::shared_ptr<Symbol> Resolve(const std::string& name) const;

// private:
    std::unordered_map<std::string, std::shared_ptr<Symbol>> store_;
    size_t next_index_ = 0;
};

}  // namespace monkey
}  // namespace pyc