#include "tiny_db/parser.h"

#include <iostream>
#include <string>

#include <fmt/core.h>

namespace tiny_db {

void Parser::PrintPrompt() { fmt::print("db > "); }

bool Parser::ParseMetaCommand(std::string_view command) {
    if (command == ".exit") {
        fmt::println("Bye!");
        exit(EXIT_SUCCESS);
    } else {
        fmt::println("Unrecognized command: {}", command);
        return true;
    }
    return false;
}

void Parser::Start() {
    while (true) {
        PrintPrompt();

        std::string input_line;
        std::getline(std::cin, input_line);

        if (ParseMetaCommand(input_line)) {
            continue;
        }
    }
}

}  // namespace tiny_db
