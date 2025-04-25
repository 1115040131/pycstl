#include "monkey/repl/repl.h"

#include <iostream>

#include <fmt/base.h>

#include "monkey/lexer/lexer.h"

namespace pyc {
namespace monkey {

void Repl::Start() {
    std::string line;
    while (true) {
        fmt::print("{}", kPrompt);

        std::getline(std::cin, line);

        if (line.empty()) {
            break;
        }

        auto lexer = Lexer::New(line);
        while (true) {
            auto token = lexer->nextToken();
            if (token.type == Token::Type::kEof) {
                break;
            }
            fmt::println("{}", token);
        }
    }
}

}  // namespace monkey
}  // namespace pyc