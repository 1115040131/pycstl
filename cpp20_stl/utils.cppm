module;

#include <string_view>

#include <fmt/base.h>

export module cpp20_stl.utils;

export namespace pyc {

void printc(const auto& container, std::string_view s = "") {
    if (!s.empty()) {
        fmt::print("{}", s.data());
    }
    for (const auto& v : container) {
        fmt::print("{} ", v);
    }
    fmt::print("\n");
}

}  // namespace pyc
