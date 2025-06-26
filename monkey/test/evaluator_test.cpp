#include <gtest/gtest.h>

#include "monkey/evaluator/evaluator.h"
#include "monkey/object/environment.h"
#include "monkey/parser/parser.h"

namespace pyc {
namespace monkey {

std::shared_ptr<Object> EvalInput(std::string_view input) {
    auto lexer = Lexer::New(input);
    auto parser = Parser::New(std::move(lexer));
    auto program = parser->parseProgram();
    if (parser->errors().size() > 0) {
        std::cerr << "Parser errors: " << parser->errorsToString() << std::endl;
        return nullptr;
    }
    auto env = Environment::New();
    return Eval(std::move(program), env);
}

#define TEST_NULL_OBJECT(object, input)                                                                        \
    {                                                                                                          \
        auto null = std::dynamic_pointer_cast<Null>(evaluated);                                                \
        ASSERT_TRUE(null != nullptr) << "Input: " << input << "\nExpected Null, got " << evaluated->typeStr(); \
    }

#define TEST_INTEGER_OBJECT(object, expected, input)                                     \
    {                                                                                    \
        auto integer = std::dynamic_pointer_cast<Integer>(evaluated);                    \
        ASSERT_TRUE(integer != nullptr)                                                  \
            << "Input: " << input << "\nExpected Integer, got " << evaluated->typeStr(); \
        EXPECT_EQ(integer->value(), expected) << "Input: " << input;                     \
    }

#define TEST_BOOLEAN_OBJECT(object, expected, input)                                           \
    {                                                                                          \
        auto boolean = std::dynamic_pointer_cast<BooleanObject>(evaluated);                    \
        ASSERT_TRUE(boolean != nullptr)                                                        \
            << "Input: " << input << "\nExpected BooleanObject, got " << evaluated->typeStr(); \
        EXPECT_EQ(boolean->value(), expected) << "Input: " << input;                           \
    }

#define TEST_STRING_OBJECT(object, expected, input)                                                             \
    {                                                                                                           \
        auto str = std::dynamic_pointer_cast<String>(evaluated);                                                \
        ASSERT_TRUE(str != nullptr) << "Input: " << input << "\nExpected String, got " << evaluated->typeStr(); \
        EXPECT_EQ(str->value(), expected) << "Input: " << input;                                                \
    }

TEST(EvaluatorTest, EvalIntegerExpression) {
    struct Input {
        std::string input;
        long long expected;
    };
    Input inputs[] = {
        {"5", 5},
        {"10", 10},
        {"-5", -5},
        {"-10", -10},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 * 2 * 2 * 2", 32},
        {"-50 + 100 + -50", 0},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"20 + 2 * -10", 0},
        {"50 / 2 * 2 + 10", 60},
        {"2 * (5 + 10)", 30},
        {"3 * 3 * 3 + 10", 37},
        {"3 * (3 * 3) + 10", 37},
        {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        TEST_INTEGER_OBJECT(evaluated, input.expected, input.input);
    }
}

TEST(EvaluatorTest, EvalBoolExpression) {
    struct Input {
        std::string input;
        bool expected;
    };
    Input inputs[] = {
        {"true", true},
        {"false", false},
        {"1 < 2", true},
        {"1 > 2", false},
        {"1 < 1", false},
        {"1 > 1", false},
        {"1 == 1", true},
        {"1 != 1", false},
        {"1 == 2", false},
        {"1 != 2", true},
        {"true == true", true},
        {"false == false", true},
        {"true == false", false},
        {"true != false", true},
        {"false != true", true},
        {"(1 < 2) == true", true},
        {"(1 < 2) == false", false},
        {"(1 > 2) == true", false},
        {"(1 > 2) == false", true},
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        TEST_BOOLEAN_OBJECT(evaluated, input.expected, input.input);
    }
}

TEST(EvaluatorTest, EvalStringExpression) {
    struct Input {
        std::string input;
        std::string expected;
    };
    Input inputs[] = {
        {"\"5\"", "5"},                      //
        {"\"10\"", "10"},                    //
        {"\"-5\"", "-5"},                    //
        {"\"-10\"", "-10"},                  //
        {"\"hello\"", "hello"},              //
        {"\"hello world\"", "hello world"},  //
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        TEST_STRING_OBJECT(evaluated, input.expected, input.input);
    }
}

TEST(EvaluatorTest, EvalStringConcat) {
    struct Input {
        std::string input;
        std::string expected;
    };
    Input inputs[] = {{"\"hello\" + \" \" + \"world!\"", "hello world!"},
                      {"\"hello world\" + \"!\"", "hello world!"}};

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        TEST_STRING_OBJECT(evaluated, input.expected, input.input);
    }
}

TEST(EvaluatorTest, EvalBangOperator) {
    struct Input {
        std::string input;
        bool expected;
    };
    Input inputs[] = {
        {"!true", false},    //
        {"!false", true},    //
        {"!0", true},        //
        {"!5", false},       //
        {"!!true", true},    //
        {"!!false", false},  //
        {"!!0", false},      //
        {"!!5", true},       //
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        TEST_BOOLEAN_OBJECT(evaluated, input.expected, input.input);
    }
}

TEST(EvaluatorTest, EvalIfElseExpressions) {
    struct Input {
        std::string input;
        std::optional<int> expected;
    };
    Input inputs[] = {
        {"if (true) { 10 }", 10},
        {"if (false) { 10 }", {}},
        {"if (1) { 10 }", 10},
        {"if (1 < 2) { 10 }", 10},
        {"if (1 > 2) { 10 }", {}},
        {"if (1 > 2) { 10 } else { 20 }", 20},
        {"if (1 < 2) { 10 } else { 20 }", 10},
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        if (input.expected) {
            TEST_INTEGER_OBJECT(evaluated, input.expected.value(), input.input);
        } else {
            TEST_NULL_OBJECT(evaluated, input.input);
        }
    }
}

TEST(EvaluatorTest, EvalReturnStatements) {
    struct Input {
        std::string input;
        int64_t expected;
    };
    Input inputs[] = {
        {"return 10;", 10},
        {"return 10; 9;", 10},
        {"return 2 * 5; 9;", 10},
        {"9; return 2 * 5; 9;", 10},
        {"if (10 > 1) { return 10; }", 10},
        {
            R""(
if (10 > 1) {
  if (10 > 1) {
    return 10;
  }

  return 1;
}
)"",
            10,
        },
        {
            R""(
let f = fn(x) {
  return x;
  x + 10;
};
f(10);)"",
            10,
        },
        {
            R""(
let f = fn(x) {
   let result = x + 10;
   return result;
   return 10;
};
f(10);)"",
            20,
        },
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        TEST_INTEGER_OBJECT(evaluated, input.expected, input.input);
    }
}

TEST(EvaluatorTest, EvalLetStatements) {
    struct Input {
        std::string input;
        int64_t expected;
    };
    Input inputs[] = {
        {"let a = 5; a;", 5},
        {"let a = 5 * 5; a;", 25},
        {"let a = 5; let b = a; b;", 5},
        {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        TEST_INTEGER_OBJECT(evaluated, input.expected, input.input);
    }
}

TEST(EvaluatorTest, EvalFunctionObject) {
    std::string input = "fn(x) { x + 2; };";
    auto evaluated = EvalInput(input);
    ASSERT_TRUE(evaluated != nullptr) << "Input: " << input;
    auto function = std::dynamic_pointer_cast<Function>(evaluated);
    ASSERT_TRUE(function != nullptr) << "Input: " << input;
    EXPECT_EQ(function->parameters().size(), 1) << "Input: " << input;
    EXPECT_EQ(function->parameters()[0]->tokenLiteral(), "x") << "Input: " << input;
    EXPECT_EQ(function->body()->toString(), "{ (x + 2) }") << "Input: " << input;
}

TEST(EvaluatorTest, EvalFunctionApplication) {
    struct Input {
        std::string input;
        int64_t expected;
    };
    Input inputs[] = {
        {"let identity = fn(x) { x; }; identity(5);", 5},
        {"let identity = fn(x) { return x; }; identity(5);", 5},
        {"let double = fn(x) { x * 2; }; double(5);", 10},
        {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
        {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
        {"fn(x) { x; }(5)", 5},
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        TEST_INTEGER_OBJECT(evaluated, input.expected, input.input);
    }
}

TEST(EvaluatorTest, EnclosingEnvironments) {
    std::string input = R""(
let first = 10;
let second = 10;
let third = 10;

let ourFunction = fn(first) {
  let second = 20;

  first + second + third;
};

ourFunction(20) + first + second;)"";

    auto evaluated = EvalInput(input);
    ASSERT_TRUE(evaluated != nullptr) << "Input: " << input;
    TEST_INTEGER_OBJECT(evaluated, 70, input);
}

TEST(EvaluatorTest, BuiltinTest) {
    struct Input {
        std::string input;
        std::variant<int, std::string> expected;
    };

    struct Input inputs[]{
        {"len(\"\")", 0},
        {"len(\"four\")", 4},
        {"len(\"hello world\")", 11},
        {"len(1)", "argument to 'len' not supported, got INTEGER"},
        {"len(\"one\", \"two\")", "wrong number of arguments. got=2, want=1"},
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        if (std::holds_alternative<int>(input.expected)) {
            TEST_INTEGER_OBJECT(evaluated, std::get<int>(input.expected), input.input);
        } else {
            auto error = std::dynamic_pointer_cast<Error>(evaluated);
            ASSERT_TRUE(error != nullptr) << "Input: " << input.input;
            EXPECT_EQ(error->inspect(), std::get<std::string>(input.expected)) << "Input: " << input.input;
        }
    }
}

TEST(EvaluatorTest, ErrorHandling) {
    struct Input {
        std::string input;
        std::string expected_message;
    };
    Input inputs[] = {
        {
            "5 + true;",
            "type mismatch: INTEGER + BOOLEAN",
        },
        {
            "5 + true; 5;",
            "type mismatch: INTEGER + BOOLEAN",
        },
        {
            "-true",
            "unknown operator: -BOOLEAN",
        },
        {
            "true + false;",
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            "true + false + true + false;",
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            "5; true + false; 5",
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            "if (10 > 1) { true + false; }",
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            R""(
if (10 > 1) {
  if (10 > 1) {
    return true + false;
  }

  return 1;
}
)"",
            "unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            "foobar",
            "identifier not found: foobar",
        },
        {
            "\"Hello\" - \"World\"",
            "unknown operator: STRING - STRING",
        },
    };

    for (const auto& input : inputs) {
        auto evaluated = EvalInput(input.input);
        ASSERT_TRUE(evaluated != nullptr) << "Input: " << input.input;
        auto error = std::dynamic_pointer_cast<Error>(evaluated);
        ASSERT_TRUE(error != nullptr) << "Input: " << input.input;
        EXPECT_EQ(error->inspect(), input.expected_message) << "Input: " << input.input;
    }
}

}  // namespace monkey
}  // namespace pyc