#include "tiny_db/parser.h"

#include <fmt/core.h>

namespace tiny_db {

bool Parser::ParseMetaCommand(std::string_view command) {
    if (command.empty()) {
        return true;
    }

    // 以 . 开头的非sql语句称作元命令
    if (command[0] == '.') {
        switch (DoMetaCommand(command)) {
            case MetaCommandResult::kSuccess:
                return true;
            case MetaCommandResult::kUnrecognizedCommand:
                fmt::println("Unrecognized command: {}", command);
                return true;
            default:
                return true;
        }
    }
    return false;
}

Parser::MetaCommandResult Parser::DoMetaCommand(std::string_view command) {
    if (command == ".exit") {
        fmt::println("Bye!");
        exit(EXIT_SUCCESS);
    }
    return MetaCommandResult::kUnrecognizedCommand;
}

bool Parser::ParseStatement(std::string_view input_line, Statement& statement) {
    switch (PrepareStatement(input_line, statement)) {
        case PrepareResult::kSuccess:
            return false;
        case PrepareResult::kUnrecognizedStatement:
            fmt::println("Unrecognized keyword at start of '{}'.", input_line);
            return true;
        default:
            return true;
    }
}

Parser::PrepareResult Parser::PrepareStatement(std::string_view input_line, Statement& statement) {
    if (input_line.starts_with("insert")) {
        statement.type = Statement::Type::kInsert;
        return PrepareResult::kSuccess;
    } else if (input_line.starts_with("select")) {
        statement.type = Statement::Type::kSelect;
        return PrepareResult::kSuccess;
    }
    return PrepareResult::kUnrecognizedStatement;
}

}  // namespace tiny_db
