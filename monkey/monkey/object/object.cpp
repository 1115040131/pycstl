#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

template <std::derived_from<Object> T>
static std::string Join(const std::vector<std::shared_ptr<T>>& nodes, std::string dim) {
    std::string connect;
    for (const auto& node : nodes) {
        connect += node->inspect() + dim;
    }
    if (!connect.empty()) {
        connect.pop_back();
        connect.pop_back();
    }
    return connect;
}

std::string_view Object::typeStr() const { return toString(type()); }

HashKey Object::getHashKey() const { return {type(), 0}; }

std::string_view toString(Object::Type type) {
    switch (type) {
        TO_STRING_CASE(Object::Type, Null);
        TO_STRING_CASE(Object::Type, ERROR);
        TO_STRING_CASE(Object::Type, INTEGER);
        TO_STRING_CASE(Object::Type, BOOLEAN);
        TO_STRING_CASE(Object::Type, STRING);
        TO_STRING_CASE(Object::Type, RETURN_VALUE);
        TO_STRING_CASE(Object::Type, FUNCTION);
        TO_STRING_CASE(Object::Type, ARRAY);
        TO_STRING_CASE(Object::Type, BUILTIN);
        default:
            return "UNKNOWN";
    }
}

std::string Function::inspect() const {
    std::string params;
    for (const auto& param : parameters_) {
        params += param->toString() + ", ";
    }
    return fmt::format("fn({}) {}", params, body_->toString());
}

std::string Array::inspect() const { return fmt::format("[{}]", Join(elements_, ", ")); }

std::string Hash::inspect() const {
    std::vector<std::string> items{};
    for (const auto& [_, pair] : pairs_) {
        items.push_back(fmt::format("{}: {}", pair.key->inspect(), pair.value->inspect()));
    }
    return fmt::format("{{{}}}", Join(items, ", "));
}

}  // namespace monkey
}  // namespace pyc