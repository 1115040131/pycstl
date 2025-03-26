#include "monkey/token/token.h"

#include <fmt/base.h>

using namespace pyc::monkey;

int main() {
    for (const auto& [keyword, type] : kKeywords) {
        fmt::print("{}: {}\n", keyword, toString(type));
    }
}