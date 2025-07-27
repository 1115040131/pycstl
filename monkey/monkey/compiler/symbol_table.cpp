#include "monkey/compiler/symbol_table.h"

namespace pyc {
namespace monkey {

std::shared_ptr<Symbol> SymbolTable::Define(std::string_view name) {
    store_[name] = std::make_shared<Symbol>(Symbol{name, SymbolScopeType::kGlobal, next_index_++});
    return store_[name];
}

std::shared_ptr<Symbol> SymbolTable::Resolve(std::string_view name) const {
    auto it = store_.find(name);
    if (it != store_.end()) {
        return it->second;
    }
    return nullptr;
}

}  // namespace monkey
}  // namespace pyc