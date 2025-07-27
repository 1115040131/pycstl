#include <gtest/gtest.h>

#include "monkey/compiler/compiler.h"
#include "monkey/test/test_define.h"

namespace pyc {
namespace monkey {

struct CompilerTestCase {
    std::string input;
    std::vector<Expected> expected_constants;
    std::vector<Instructions> expected_instructions;
};

Instructions concateInstructions(const std::vector<Instructions>& instructions) {
    Instructions concated;
    for (const auto& instruction : instructions) {
        concated.insert(concated.end(), instruction.begin(), instruction.end());
    }
    return concated;
}

#define TEST_INSTRUCTIONS(expected, actual, input)                                     \
    {                                                                                  \
        auto concated = concateInstructions(expected);                                 \
        ASSERT_EQ(concated.size(), actual.size());                                     \
        for (size_t i = 0; i < concated.size(); i++) {                                 \
            EXPECT_EQ(concated[i], actual[i]) << "Input: " << input << "\nconcated:\n" \
                                              << toString(concated) << "actual:\n"     \
                                              << toString(actual);                     \
        }                                                                              \
    }

#define TEST_CONSTANTS(expected, actual, input)                  \
    {                                                            \
        ASSERT_EQ(expected.size(), actual.size());               \
        for (size_t i = 0; i < expected.size(); i++) {           \
            TEST_EXPECTED_OBJECT(actual[i], expected[i], input); \
        }                                                        \
    }

#define RUN_COMPILER_TESTS(tests)                                                            \
    for (const auto& test : tests) {                                                         \
        auto compiler = Compiler::New();                                                     \
        auto err = compiler->compile(processInput(test.input));                              \
        ASSERT_FALSE(err) << "Input: " << test.input;                                        \
        TEST_INSTRUCTIONS(test.expected_instructions, compiler->instructions(), test.input); \
        TEST_CONSTANTS(test.expected_constants, compiler->constants(), test.input);          \
    }

TEST(CompilerTest, IntegerArithmeticTest) {
    CompilerTestCase tests[] = {
        {"1+2",
         {1, 2},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpAdd, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"1;2",
         {1, 2},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpPop, {}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"1-2",
         {1, 2},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpSub, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"1*2",
         {1, 2},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpMul, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"1 / 2",
         {1, 2},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpDiv, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"2 / 1",
         {2, 1},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpDiv, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"-1",
         {1},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpMinus, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
    };

    RUN_COMPILER_TESTS(tests);
}

TEST(CompilerTest, BooleanExpressionTest) {
    CompilerTestCase tests[] = {
        {"true",
         {},
         {
             ByteCode::Make(OpcodeType::OpTrue, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"false",
         {},
         {
             ByteCode::Make(OpcodeType::OpFalse, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"1 > 2",
         {1, 2},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpGreaterThan, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"1 < 2",
         {2, 1},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpGreaterThan, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"1 == 2",
         {1, 2},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpEqual, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"1 != 2",
         {1, 2},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpNotEqual, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"true == false",
         {},
         {
             ByteCode::Make(OpcodeType::OpTrue, {}),
             ByteCode::Make(OpcodeType::OpFalse, {}),
             ByteCode::Make(OpcodeType::OpEqual, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"true != false",
         {},
         {
             ByteCode::Make(OpcodeType::OpTrue, {}),
             ByteCode::Make(OpcodeType::OpFalse, {}),
             ByteCode::Make(OpcodeType::OpNotEqual, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"!true",
         {},
         {
             ByteCode::Make(OpcodeType::OpTrue, {}),
             ByteCode::Make(OpcodeType::OpBang, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
    };

    RUN_COMPILER_TESTS(tests);
}

TEST(CompilerTest, StringTest) {
    CompilerTestCase tests[] = {
        {R"("monkey")",
         {"monkey"},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {R"("mon" + "key")",
         {"mon", "key"},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpAdd, {}),
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
    };

    RUN_COMPILER_TESTS(tests);
}

TEST(CompilerTest, ArrayLiteralTest) {
    CompilerTestCase tests[] = {
        {"[]",
         {},
         {
             {ByteCode::Make(OpcodeType::OpArray, {0})},
             {ByteCode::Make(OpcodeType::OpPop, {})},
         }},
        {"[1, 2, 3]",
         {1, 2, 3},
         {
             {ByteCode::Make(OpcodeType::OpConstant, {0})},
             {ByteCode::Make(OpcodeType::OpConstant, {1})},
             {ByteCode::Make(OpcodeType::OpConstant, {2})},
             {ByteCode::Make(OpcodeType::OpArray, {3})},
             {ByteCode::Make(OpcodeType::OpPop, {})},
         }},
        {"[1+2, 3-4, 5*6]",
         {1, 2, 3, 4, 5, 6},
         {
             {ByteCode::Make(OpcodeType::OpConstant, {0})},
             {ByteCode::Make(OpcodeType::OpConstant, {1})},
             {ByteCode::Make(OpcodeType::OpAdd, {})},
             {ByteCode::Make(OpcodeType::OpConstant, {2})},
             {ByteCode::Make(OpcodeType::OpConstant, {3})},
             {ByteCode::Make(OpcodeType::OpSub, {})},
             {ByteCode::Make(OpcodeType::OpConstant, {4})},
             {ByteCode::Make(OpcodeType::OpConstant, {5})},
             {ByteCode::Make(OpcodeType::OpMul, {})},
             {ByteCode::Make(OpcodeType::OpArray, {3})},
             {ByteCode::Make(OpcodeType::OpPop, {})},
         }},
    };

    RUN_COMPILER_TESTS(tests);
}

TEST(CompilerTest, HashLiteralTest) {
    CompilerTestCase tests[] = {
        {"{}",
         {},
         {
             {ByteCode::Make(OpcodeType::OpHash, {0})},
             {ByteCode::Make(OpcodeType::OpPop, {})},
         }},
        {"{1:2, 3:4, 5:6}",
         {1, 2, 3, 4, 5, 6},
         {
             {ByteCode::Make(OpcodeType::OpConstant, {0})},
             {ByteCode::Make(OpcodeType::OpConstant, {1})},
             {ByteCode::Make(OpcodeType::OpConstant, {2})},
             {ByteCode::Make(OpcodeType::OpConstant, {3})},
             {ByteCode::Make(OpcodeType::OpConstant, {4})},
             {ByteCode::Make(OpcodeType::OpConstant, {5})},
             {ByteCode::Make(OpcodeType::OpHash, {3})},
             {ByteCode::Make(OpcodeType::OpPop, {})},
         }},
        {"{1: 2+3, 4:5*6}",
         {1, 2, 3, 4, 5, 6},
         {
             {ByteCode::Make(OpcodeType::OpConstant, {0})},
             {ByteCode::Make(OpcodeType::OpConstant, {1})},
             {ByteCode::Make(OpcodeType::OpConstant, {2})},
             {ByteCode::Make(OpcodeType::OpAdd, {})},
             {ByteCode::Make(OpcodeType::OpConstant, {3})},
             {ByteCode::Make(OpcodeType::OpConstant, {4})},
             {ByteCode::Make(OpcodeType::OpConstant, {5})},
             {ByteCode::Make(OpcodeType::OpMul, {})},
             {ByteCode::Make(OpcodeType::OpHash, {2})},
             {ByteCode::Make(OpcodeType::OpPop, {})},
         }},
    };

    RUN_COMPILER_TESTS(tests);
}

TEST(CompilerTest, IfExpressionTest) {
    CompilerTestCase tests[] = {
        {"if( true ){ 10 }; 3333;",
         {10, 3333},
         {
             // 0000
             ByteCode::Make(OpcodeType::OpTrue, {}),
             // 0001
             ByteCode::Make(OpcodeType::OpJumpNotTruthy, {10}),
             // 0004
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             // 0007
             ByteCode::Make(OpcodeType::OpJump, {11}),
             // 0010
             ByteCode::Make(OpcodeType::OpNull, {}),
             // 0011
             ByteCode::Make(OpcodeType::OpPop, {}),
             // 0012
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             // 0015
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
        {"if( true ){ 10 } else { 20 }; 3333;",
         {10, 20, 3333},
         {
             // 0000
             ByteCode::Make(OpcodeType::OpTrue, {}),
             // 0001
             ByteCode::Make(OpcodeType::OpJumpNotTruthy, {10}),
             // 0004
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             // 0007
             ByteCode::Make(OpcodeType::OpJump, {13}),
             // 0010
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             // 0013
             ByteCode::Make(OpcodeType::OpPop, {}),
             // 0014
             ByteCode::Make(OpcodeType::OpConstant, {2}),
             // 0017
             ByteCode::Make(OpcodeType::OpPop, {}),
         }},
    };

    RUN_COMPILER_TESTS(tests);
}

TEST(CompilerTest, CompileGlobalStatementsTest) {
    CompilerTestCase tests[] = {
        {"let one = 1; let two = 2;",
         {1, 2},
         {
             ByteCode::Make(OpcodeType::OpConstant, {0}),
             ByteCode::Make(OpcodeType::OpSetGlobal, {0}),
             ByteCode::Make(OpcodeType::OpConstant, {1}),
             ByteCode::Make(OpcodeType::OpSetGlobal, {1}),
         }},
        {"let one = 1; one;",
         {1},
         {
             {ByteCode::Make(OpcodeType::OpConstant, {0})},
             {ByteCode::Make(OpcodeType::OpSetGlobal, {0})},
             {ByteCode::Make(OpcodeType::OpGetGlobal, {0})},
             {ByteCode::Make(OpcodeType::OpPop, {})},
         }},
    };

    RUN_COMPILER_TESTS(tests);
}

}  // namespace monkey
}  // namespace pyc
