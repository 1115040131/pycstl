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

std::shared_ptr<Symbol> SymbolTable::Resolve(const std::string& name) const {
    auto it = store_.find(name);
    if (it != store_.end()) {
        return it->second;
    } else if (outer_) {
        return outer_->Resolve(name);
    }
    return nullptr;
}

}  // namespace monkey
}  // namespace pyc