#include "monkey/compiler/symbol_table.h"

namespace pyc {
namespace monkey {

std::shared_ptr<Symbol> SymbolTable::Define(const std::string& name) {
    auto symbol = std::make_shared<Symbol>(Symbol{name, SymbolScopeType::kGlobal, next_index_++});

    if (outer_) {
        symbol->scope = SymbolScopeType::kLocal;
    }
    store_[name] = symbol;
    return symbol;
}

std::shared_ptr<Symbol> SymbolTable::DefineBuiltin(const std::string& name, size_t index) {
    auto symbol = std::make_shared<Symbol>(Symbol{name, SymbolScopeType::kBuiltin, index});
    store_[name] = symbol;
    return symbol;
}

std::shared_ptr<Symbol> SymbolTable::DefineFree(std::shared_ptr<Symbol> original) {
    free_symbols_.push_back(original);
    auto symbol =
        std::make_shared<Symbol>(Symbol{original->name, SymbolScopeType::kFree, free_symbols_.size() - 1});
    store_[original->name] = symbol;
    return symbol;
}

std::shared_ptr<Symbol> SymbolTable::Resolve(const std::string& name) {
    auto it = store_.find(name);
    if (it != store_.end()) {
        return it->second;
    } else if (outer_) {
        auto object = outer_->Resolve(name);

        if (!object || object->scope == SymbolScopeType::kGlobal || object->scope == SymbolScopeType::kBuiltin) {
            return object;
        }

        return DefineFree(object);
    }
    return nullptr;
}

}  // namespace monkey
}  // namespace pyc