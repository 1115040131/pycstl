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
    ExecuteResult result;
    switch (statement.type) {
        case Statement::Type::kInsert:
            result = ExecuteInsert(statement);
            break;
        case Statement::Type::kSelect:
            result = ExecuteSelect();
            break;
        default:
            break;
    }

    switch (result) {
        case ExecuteResult::kSuccess:
            fmt::println("Executed.");
            break;
        case ExecuteResult::kTableFull:
            fmt::println("Error: Table full.");
            break;
        default:
            break;
    }
}

Machine::ExecuteResult Machine::ExecuteInsert(const Statement& statement) {
    if (table_.num_rows >= kTableMaxRows) {
        return ExecuteResult::kTableFull;
    }

    table_.GetRow(table_.num_rows) = statement.row_to_insert;
    table_.num_rows++;
    return ExecuteResult::kSuccess;
}

Machine::ExecuteResult Machine::ExecuteSelect() {
    for (uint32_t i = 0; i < table_.num_rows; i++) {
        fmt::println("{}", table_.GetRow(i).ToString());
    }
    return ExecuteResult::kSuccess;
}

}  // namespace tiny_db
