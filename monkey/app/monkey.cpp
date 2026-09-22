#include <fmt/base.h>

import monkey.repl;

using namespace pyc::monkey;

int main() {
    fmt::println("This is the Monkey-CPP programming language!");
    fmt::println("Feel free to type in commands");
    Repl::Start();
}
