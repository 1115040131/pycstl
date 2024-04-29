#pragma once

#include <string_view>

namespace tiny_db {

class Parser {
public:
    void PrintPrompt();
    bool ParseMetaCommand(std::string_view command);
    void Start();
};

}  // namespace tiny_db
