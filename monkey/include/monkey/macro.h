#pragma once

namespace pyc {
namespace monkey {

#define TYPE(x) \
    virtual Type type() const override { return Type::x; }

#define TO_STRING_CASE(class_name, x) \
    case class_name::x:               \
        return #x

#define TO_STRING_CASE1(class_name, x, str) \
    case class_name::x:                     \
        return str

}  // namespace monkey
}  // namespace pyc