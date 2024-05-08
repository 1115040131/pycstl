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
    MetaCommandResult DoMetaCommand(std::string_view command);

    enum class PrepareResult {
        kSuccess,
        kNegtiveId,
        kStringTooLong,
        kSyntaxError,

        kUnrecognizedStatement,
    };
    PrepareResult PrepareStatement(std::string_view input_line, Statement& statement);
    PrepareResult PrepareInsert(std::string_view input_line, Statement& statement);
};

}  // namespace tiny_db
