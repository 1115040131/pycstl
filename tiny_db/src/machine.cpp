#include "tiny_db/machine.h"

#include <iostream>
#include <sstream>
#include <string>

#include <fmt/core.h>

namespace tiny_db {

void Machine::PrintPrompt() { fmt::print("db > "); }

void Machine::Start() {
    while (true) {
        PrintPrompt();

        std::string input_line;
        std::getline(std::cin, input_line);

        if (ParseMetaCommand(input_line)) {
            continue;
        }

        Statement statement;
        if (ParseStatement(input_line, statement)) {
            continue;
        }

        ExecuteStatement(statement);
    }
}

#pragma region 解析元命令

bool Machine::ParseMetaCommand(std::string_view command) {
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

Machine::MetaCommandResult Machine::DoMetaCommand(std::string_view command) {
    if (command == ".exit") {
        table_.reset();
        fmt::println("Bye!");
        exit(EXIT_SUCCESS);
    } else if (command == ".btree") {
        fmt::println("Tree:");
        table_->RootPage().Print();
        return MetaCommandResult::kSuccess;
    } else if (command == ".constants") {
        fmt::println("Constants:");
        fmt::println("  kRowSize: {}", kRowSize);
        fmt::println("  kHeadSize: {}", Table::DataType::kHeadSize);
        fmt::println("  kCellSize: {}", Table::DataType::kCellSize);
        fmt::println("  kSpaceForCells: {}", kPageSize - Table::DataType::kHeadSize);
        fmt::println("  kMaxCells: {}", Table::DataType::kMaxCells);
        return MetaCommandResult::kSuccess;
    }
    return MetaCommandResult::kUnrecognizedCommand;
}

#pragma endregion

#pragma region 解析SQL语句

bool Machine::ParseStatement(std::string_view input_line, Statement& statement) {
    switch (PrepareStatement(input_line, statement)) {
        case PrepareResult::kSuccess:
            return false;
        case PrepareResult::kNegtiveId:
            fmt::println("ID must be positive.");
            return true;
        case PrepareResult::kStringTooLong:
            fmt::println("String is too long.");
            return true;
        case PrepareResult::kSyntaxError:
            fmt::println("Syntax error. Could not parse statement.");
            return true;
        case PrepareResult::kUnrecognizedStatement:
            fmt::println("Unrecognized keyword at start of '{}'.", input_line);
            return true;
        default:
            return true;
    }
}

Machine::PrepareResult Machine::PrepareStatement(std::string_view input_line, Statement& statement) {
    if (input_line.starts_with("insert")) {
        return PrepareInsert(input_line, statement);
    } else if (input_line.starts_with("select")) {
        statement.type = Statement::Type::kSelect;
        return PrepareResult::kSuccess;
    }
    return PrepareResult::kUnrecognizedStatement;
}

Machine::PrepareResult Machine::PrepareInsert(std::string_view input_line, Statement& statement) {
    statement.type = Statement::Type::kInsert;

    std::istringstream iss(input_line.data());
    std::string keyword;
    std::string id_string;
    std::string username;
    std::string email;

    // 从输入串流提取关键字和参数
    if (!(iss >> keyword >> id_string >> username >> email)) {
        return PrepareResult::kSyntaxError;
    }

    // 将 ID 字符串转换为整数，并检查是否为负数
    int id;
    try {
        id = std::stoi(id_string);
        if (id < 0) {
            return PrepareResult::kNegtiveId;
        }
    } catch (const std::invalid_argument& e) {
        // 如果无法将字符串转换为整数
        return PrepareResult::kSyntaxError;
    } catch (const std::out_of_range& e) {
        // 如果整数超出了 stoi 可以处理的范围
        return PrepareResult::kSyntaxError;
    }

    // 检查 username 和 email 长度是否超出限制
    if (username.size() > kUsernameSize) {
        return PrepareResult::kStringTooLong;
    }
    if (email.size() > kEmailSize) {
        return PrepareResult::kStringTooLong;
    }

    statement.row_to_insert.id = id;
    strncpy(statement.row_to_insert.username, username.c_str(), kUsernameSize);
    strncpy(statement.row_to_insert.email, email.c_str(), kEmailSize);
    return PrepareResult::kSuccess;
}

#pragma endregion

#pragma region 执行SQL语句

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
    if (table_->Full()) {
        return ExecuteResult::kTableFull;
    }

    table_->Insert(table_->end(), statement.row_to_insert);
    return ExecuteResult::kSuccess;
}

Machine::ExecuteResult Machine::ExecuteSelect() {
    for (const auto& row : *table_) {
        fmt::println("{}", row.value.ToString());
    }
    return ExecuteResult::kSuccess;
}

#pragma endregion

}  // namespace tiny_db
