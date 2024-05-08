#pragma once

#include "tiny_db/statement.h"
#include "tiny_db/table.h"

namespace tiny_db {

class Machine {
public:
    void Start();

private:
    void PrintPrompt();

    void ExecuteStatement(Statement& statement);

    enum class ExecuteResult {
        kSuccess,
        kTableFull,
    };
    ExecuteResult ExecuteInsert(const Statement& statement);
    ExecuteResult ExecuteSelect();

private:
    Table table_;
};

}  // namespace tiny_db
