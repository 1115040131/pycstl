#pragma once

#include <string>
#include <string_view>

namespace pyc {

/// @brief 从带有作用域的函数名中删除命名空间和类名
std::string_view GetSimpleName(std::string_view functionName);

/// @brief 拼接 "host:port" 形式的地址，常见长度下命中 SSO 不产生堆分配
std::string JoinHostPort(std::string_view host, std::string_view port);

}  // namespace pyc