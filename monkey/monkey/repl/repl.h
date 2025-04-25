#pragma once

#include <string_view>

namespace pyc {
namespace monkey {

class Repl {
public:
    static void Start();

private:
    static constexpr std::string_view kPrompt = ">> ";
};

}  // namespace monkey
}  // namespace pyc