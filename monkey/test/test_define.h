#pragma once

#include <iostream>

#include "monkey/token/token.h"

namespace pyc {
namespace monkey {

inline bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.literal == rhs.literal;
}

inline std::ostream& operator<<(std::ostream& os, const Token& token) {
    return os << "Token{type: \"" << toString(token.type) << "\", literal: \"" << token.literal << "\"}";
}

}  // namespace monkey
}  // namespace pyc