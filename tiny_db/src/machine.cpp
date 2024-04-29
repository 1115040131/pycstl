#include "tiny_db/machine.h"

#include <iostream>
#include <string>

#include <fmt/core.h>

#include "tiny_db/parser.h"

namespace tiny_db {

void Machine::PrintPrompt() { fmt::print("db > "); }

void Machine::Start() {
    Parser parser;

    while (true) {
        PrintPrompt();

        std::string input_line;
        std::getline(std::cin, input_line);

        if (parser.ParseMetaCommand(input_line)) {
            continue;
        }

        Statement statement;
        if (parser.ParseStatement(input_line, statement)) {
            continue;
        }

        ExecuteStatement(statement);
    }
}

void Machine::ExecuteStatement(Statement& statement) {
    switch (statement.type) {
        case Statement::Type::kInsert:
            fmt::println("Executing insert statement");
            break;
        case Statement::Type::kSelect:
            fmt::println("Executing select statement");
            break;
        default:
            break;
    }
}

}  // namespace tiny_db
