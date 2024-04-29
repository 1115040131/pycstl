#pragma once

#include "tiny_db/statement.h"

namespace tiny_db {

class Machine {
public:
    void Start();

private:
    void PrintPrompt();

    void ExecuteStatement(Statement& statement);
};

}  // namespace tiny_db
