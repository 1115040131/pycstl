#pragma once

#include <string_view>
#include <type_traits>  // 用于 std::underlying_type

namespace pyc {

/// @brief 转换枚举到其底层类型的通用函数
template <typename E>
constexpr auto ToUnderlying(E e) -> typename std::underlying_type<E>::type {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

/// @brief 从带有作用域的函数名中删除命名空间和类名
std::string_view GetSimpleName(std::string_view functionName);

}  // namespace pyc