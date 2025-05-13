#pragma once

namespace pyc {
namespace monkey {

#define TYPE(x) \
    virtual Type type() const override { return Type::x; }

#define TO_STRING_CASE(class_name, x) \
    case class_name::x:               \
        return #x

}  // namespace monkey
}  // namespace pyc