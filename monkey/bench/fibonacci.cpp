#include <chrono>
#include <string_view>

#include <fmt/base.h>

#define STRIP_FLAG_HELP 1
#include <gflags/gflags.h>

#include "monkey/evaluator/evaluator.h"
#include "monkey/lexer/lexer.h"
#include "monkey/object/environment.h"
#include "monkey/parser/parser.h"
#include "monkey/vm/vm.h"

using namespace pyc::monkey;

constexpr std::string_view input = R""(
let fibonacci = fn(x){
    if(x == 0){
        return 0;
    } else {
        if(x == 1){
            return 1;
        } else {
            return fibonacci(x - 1) + fibonacci(x - 2);
        }
    }
};

fibonacci(35);
)"";

constexpr std::string_view input2 = "fibonacci(35);";

DEFINE_string(engine, ":)", "use 'vm' or 'eval'");
DEFINE_bool(builtin, false, "use builtin fibonacci function");

int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    auto result = std::make_shared<Object>();

    auto start = std::chrono::high_resolution_clock::now();
    auto end = start;

    auto lexer = FLAGS_builtin ? Lexer::New(input2) : Lexer::New(input);
    auto parser = Parser::New(std::move(lexer));
    auto program = parser->parseProgram();

    if (FLAGS_engine == "vm") {
        auto compiler = Compiler::New();
        if (auto result = compiler->compile(std::move(program)); IsError(result)) {
            fmt::println("Woops! Compilation failed: \n{}", result->inspect());
            return -1;
        }

        auto vm = VM::New(compiler);

        start = std::chrono::high_resolution_clock::now();

        result = vm->run();
        if (IsError(result)) {
            fmt::println("Woops! Executing bytecode failed: \n{}", result->inspect());
            return -1;
        }

        end = std::chrono::high_resolution_clock::now();

        result = vm->lastPoppedElement();
    } else if (FLAGS_engine == "eval") {
        auto env = Environment::New();

        start = std::chrono::high_resolution_clock::now();

        result = Eval(std::move(program), env);

        end = std::chrono::high_resolution_clock::now();
    } else {
        fmt::println("usage: fibonacci -engine vm|eval [-builtin]");
        return -1;
    }

    std::chrono::duration<double> diff = end - start;
    fmt::println("engine={}, fibonacci(35)={}, duration={}s", FLAGS_engine, result->inspect(), diff.count());
}