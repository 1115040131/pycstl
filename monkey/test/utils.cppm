module;

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

export module monkey.test.utils;

export import monkey.ast;
export import monkey.code;
export import monkey.token;
import monkey.lexer;
import monkey.parser;

export namespace pyc::monkey {

inline bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.literal == rhs.literal;
}

inline std::ostream& operator<<(std::ostream& os, const Token& token) {
    return os << "Token{type: \"" << toString(token.type) << "\", literal: \"" << token.literal << "\"}";
}

using Expected = std::variant<int, bool, std::string, std::vector<Instructions>, void*>;

inline std::unique_ptr<Node> processInput(std::string_view input) {
    auto lexer = Lexer::New(input);
    auto parser = Parser::New(std::move(lexer));
    auto program = parser->parseProgram();
    if (parser->errors().size() > 0) {
        std::cerr << "Parser errors: " << parser->errorsToString() << std::endl;
        return nullptr;
    }
    return program;
}

inline Instructions concateInstructions(const std::vector<Instructions>& instructions) {
    Instructions concated;
    for (const auto& instruction : instructions) {
        concated.insert(concated.end(), instruction.begin(), instruction.end());
    }
    return concated;
}

}  // namespace pyc::monkey
