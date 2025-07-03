#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

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

std::string Array::inspect() const {
    std::string elements;
    for (const auto& element : elements_) {
        elements += element->inspect() + ", ";
    }
    if (!elements.empty()) {
        elements.pop_back();  // 去掉最后的逗号
        elements.pop_back();  // 去掉最后的空格
    }
    return fmt::format("[{}]", elements);
}

}  // namespace monkey
}  // namespace pyc