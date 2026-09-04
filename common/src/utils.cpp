#include "common/utils.h"

#include <format>

namespace pyc {

std::string_view GetSimpleName(std::string_view functionName) {
    std::size_t end = functionName.find('(');  // 找到函数参数列表的开始
    if (end != std::string_view::npos) {
        // 我们需要在 "(" 之前找到 "::"
        std::size_t start = functionName.rfind("::", end);
        if (start != std::string_view::npos) {
            return functionName.substr(start + 2, end - (start + 2));  // Skip the "::"
        }
        // 没有找到 "::" 则在 "(" 之前找到 " "
        start = functionName.rfind(" ", end);
        if (start != std::string_view::npos) {
            return functionName.substr(start + 1, end - (start + 1));  // Skip the " "
        }
        // 如果没有找到 "::" 或者 " "，则从字符串开头到 "(" 为止就是函数名
        return functionName.substr(0, end);
    }
    return functionName;  // 如果没有找到 "(", 则返回原始字符串
}

std::string JoinHostPort(std::string_view host, std::string_view port) {
    // IPv6 字面量自身含冒号，必须加方括号才能和 port 的分隔符区分开；已带括号则原样保留
    if (host.find(':') != std::string_view::npos && !(host.starts_with('[') && host.ends_with(']'))) {
        return std::format("[{}]:{}", host, port);
    }
    return std::format("{}:{}", host, port);
}

}  // namespace pyc