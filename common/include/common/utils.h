#pragma once

#include <string_view>

namespace pyc {

/// @brief 从带有作用域的函数名中删除命名空间和类名
std::string_view GetSimpleName(std::string_view functionName);

}  // namespace pyc