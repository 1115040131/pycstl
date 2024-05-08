#include "tiny_db/parser.h"

#include <sstream>

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

Parser::PrepareResult Parser::PrepareStatement(std::string_view input_line, Statement& statement) {
    if (input_line.starts_with("insert")) {
        return PrepareInsert(input_line, statement);
    } else if (input_line.starts_with("select")) {
        statement.type = Statement::Type::kSelect;
        return PrepareResult::kSuccess;
    }
    return PrepareResult::kUnrecognizedStatement;
}

Parser::PrepareResult Parser::PrepareInsert(std::string_view input_line, Statement& statement) {
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

}  // namespace tiny_db
