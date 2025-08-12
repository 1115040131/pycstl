#pragma once

#include <string_view>

namespace pyc {

// 更简洁的哈希和比较函数实现
struct StringHash {
    using is_transparent = void;  // 标记支持异构查找

    std::size_t operator()(std::string_view sv) const { return std::hash<std::string_view>{}(sv); }
};

struct StringEqual {
    using is_transparent = void;  // 标记支持异构查找

    bool operator()(std::string_view lhs, std::string_view rhs) const { return lhs == rhs; }
};

}  // namespace pyc
