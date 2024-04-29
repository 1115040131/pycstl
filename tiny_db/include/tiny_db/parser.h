#pragma once

#include <optional>
#include <string_view>

#include "tiny_db/statement.h"

namespace tiny_db {

class Parser {
public:
    bool ParseMetaCommand(std::string_view command);

    bool ParseStatement(std::string_view input_line, Statement& statement);

private:
    enum class MetaCommandResult {
        kSuccess,
        kUnrecognizedCommand,
    };

    enum class PrepareResult {
        kSuccess,
        kUnrecognizedStatement,
    };

    MetaCommandResult DoMetaCommand(std::string_view command);

    PrepareResult PrepareStatement(std::string_view input_line, Statement& statement);
};

}  // namespace tiny_db
