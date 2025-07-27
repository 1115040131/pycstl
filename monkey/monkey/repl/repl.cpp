#include "monkey/repl/repl.h"

#include <iostream>

#include <fmt/base.h>

#include "monkey/evaluator/evaluator.h"
#include "monkey/lexer/lexer.h"
// #include "monkey/object/environment.h"
#include "monkey/parser/parser.h"
#include "monkey/vm/vm.h"

namespace pyc {
namespace monkey {

inline constexpr std::string_view kPrompt = ">> ";

inline constexpr std::string_view kMonkeyFace = R""(
   .--.  .-"     "-.  .--.
  / .. \/  .-. .-.  \/ .. \
 | |  '|  /   Y   \  |'  | |
 | \   \  \ 0 | 0 /  /   / |
  \ '- ,\.-"""""""-./, -' /
   ''-' /_   ^ ^   _\ '-''
       |  \._   _./  |
       \   \ '~' /   /
        '._ '-=-' _.'
           '-----'
                              )"";

void printParserErrors(const std::vector<std::string>& errors) {
    fmt::println("{}", kMonkeyFace);
    fmt::println("Woops! We ran into some monkey business here!");
    fmt::println(" parser errors:");
    for (const auto& error : errors) {
        fmt::println("\t{}", error);
    }
}

void Repl::Start() {
    // std::vector<std::string> lines;
    // lines.reserve(10);
    std::string line;
    // auto env = Environment::New();

    std::vector<std::shared_ptr<Object>> constants;
    std::vector<std::shared_ptr<Object>> globals(VM::kGlobalSize);
    auto symbol_table = SymbolTable::New();

    while (true) {
        fmt::print("{}", kPrompt);

        // lines.emplace_back();
        // auto& line = lines.back();

        std::getline(std::cin, line);

        if (line.empty()) {
            continue;
        }

        auto lexer = Lexer::New(line);
        auto parser = Parser::New(std::move(lexer));
        auto program = parser->parseProgram();

        if (!parser->errors().empty()) {
            printParserErrors(parser->errors());
            continue;
        }

        // auto evaluated = Eval(std::move(program), env);
        // if (evaluated) {
        //     fmt::println("{}", evaluated->inspect());
        // }

        auto compiler = Compiler::NewWithState(constants, symbol_table);
        if (auto result = compiler->compile(std::move(program)); IsError(result)) {
            fmt::println("Woops! Compilation failed: \n{}", result->inspect());
            continue;
        }

        auto vm = VM::NewWithState(compiler, globals);
        if (auto result = vm->run(); IsError(result)) {
            fmt::println("Woops! Executing bytecode failed: \n{}", result->inspect());
            continue;
        }

        fmt::println("{}", vm->lastPoppedElement()->inspect());

        constants = compiler->constants();
        globals = vm->globals();
    }
}

}  // namespace monkey
}  // namespace pyc