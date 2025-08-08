#pragma once

#include <string>
#include <vector>

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

inline std::string Join(const std::vector<std::string>& nodes, std::string dim) {
    std::string connect;
    for (const auto& node : nodes) {
        connect += node + dim;
    }
    if (!connect.empty()) {
        connect.pop_back();
        connect.pop_back();
    }
    return connect;
}

}  // namespace monkey
}  // namespace pyc