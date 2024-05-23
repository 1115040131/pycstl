#pragma once

#include <memory>

#include "tiny_db/statement.h"
#include "tiny_db/table.h"

namespace tiny_db {

class Machine {
public:
    Machine(std::string_view filename) : table_(std::make_unique<Table>(filename)) {}

    void Start();

private:
    void PrintPrompt();

    // 解析命令
    bool ParseMetaCommand(std::string_view command);
    enum class MetaCommandResult {
        kSuccess,
        kUnrecognizedCommand,
    };
    MetaCommandResult DoMetaCommand(std::string_view command);

    // 解析SQL语句
    bool ParseStatement(std::string_view input_line, Statement& statement);
    enum class PrepareResult {
        kSuccess,
        kNegtiveId,
        kStringTooLong,
        kSyntaxError,

        kUnrecognizedStatement,
    };
    PrepareResult PrepareStatement(std::string_view input_line, Statement& statement);
    PrepareResult PrepareInsert(std::string_view input_line, Statement& statement);

    // 执行SQL语句
    void ExecuteStatement(Statement& statement);
    enum class ExecuteResult {
        kSuccess,
        kTableFull,
    };
    ExecuteResult ExecuteInsert(const Statement& statement);
    ExecuteResult ExecuteSelect();

private:
    std::unique_ptr<Table> table_;
};

}  // namespace tiny_db
