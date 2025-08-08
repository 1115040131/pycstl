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
        TO_STRING_CASE(Object::Type, HASH);
        TO_STRING_CASE(Object::Type, BUILTIN);
        TO_STRING_CASE(Object::Type, COMPILED_FUNCTION);
        TO_STRING_CASE(Object::Type, CLOSURE);
    }
    return "Unknown Object Type";
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

bool IsTruthy(const std::shared_ptr<Object>& obj) {
    if (!obj) {
        return false;
    }
    if (obj->type() == Object::Type::BOOLEAN) {
        return std::dynamic_pointer_cast<BooleanObject>(obj)->value();
    } else if (obj->type() == Object::Type::INTEGER) {
        return std::dynamic_pointer_cast<Integer>(obj)->value() != 0;
    } else if (obj->type() == Object::Type::Null) {
        return false;
    }
    return true;
}

std::shared_ptr<BooleanObject> EvalBool(bool value) {
    if (value) {
        return kTrueObj;
    }
    return kFalseObj;
}

std::shared_ptr<Object> EvalArrayIndex(std::shared_ptr<Array> array, std::shared_ptr<Integer> index) {
    if (index->value() < 0 || index->value() >= static_cast<long long>(array->elements().size())) {
        return kNullObj;
    }
    return array->elements()[index->value()];
}

std::shared_ptr<Object> EvalHashIndex(std::shared_ptr<Hash> hash, std::shared_ptr<Object> index) {
    if (!index->hashable()) {
        return std::make_shared<Error>(fmt::format("unusable as hash key: {}", index->typeStr()));
    }
    auto iter = hash->pairs().find(index->getHashKey());
    if (iter == hash->pairs().end()) {
        return kNullObj;  // Return null if the key is not found
    }
    return iter->second.value;  // Return the value associated with the key
}

}  // namespace monkey
}  // namespace pyc