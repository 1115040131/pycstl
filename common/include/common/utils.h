#pragma once

#include <string>
#include <string_view>

namespace pyc {

/// @brief 从带有作用域的函数名中删除命名空间和类名
std::string_view GetSimpleName(std::string_view functionName);

/// @brief 拼接 gRPC target，形如 "host:port"；host 是 IPv6 字面量时补上方括号，如 "[::1]:80"
/// @note host 已经带方括号时不重复添加，可安全重复调用
std::string JoinHostPort(std::string_view host, std::string_view port);

}  // namespace pyc