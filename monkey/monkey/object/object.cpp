#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

std::string_view Object::typeStr() const { return toString(type()); }

std::string_view toString(Object::Type type) {
    switch (type) {
        TO_STRING_CASE(Object::Type, Null);
        TO_STRING_CASE(Object::Type, ERROR);
        TO_STRING_CASE(Object::Type, INTEGER);
        TO_STRING_CASE(Object::Type, BOOLEAN);
        TO_STRING_CASE(Object::Type, RETURN_VALUE);
        TO_STRING_CASE(Object::Type, FUNCTION);
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

}  // namespace monkey
}  // namespace pyc