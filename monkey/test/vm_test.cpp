#include <gtest/gtest.h>

#include "monkey/test/test_define.h"
#include "monkey/vm/vm.h"

namespace pyc {
namespace monkey {

struct VMTestCase {
    std::string input;
    Expected expected;
};

#define RUN_VM_TESTS(tests)                                                                  \
    for (const auto& test : tests) {                                                         \
        auto compiler = Compiler::New();                                                     \
        auto err = compiler->compile(processInput(test.input));                              \
        ASSERT_FALSE(err) << "Input: " << test.input;                                        \
        auto vm = VM::New(compiler);                                                         \
        auto result = vm->run();                                                             \
        ASSERT_FALSE(result) << "Input: " << test.input << "\nError: " << result->inspect(); \
        TEST_EXPECTED_OBJECT(vm->lastPoppedElement(), test.expected, test.input);            \
    }

TEST(VMTest, IntegerArithmeticTest) {
    VMTestCase tests[] = {
        {"1", 1},
        {"2", 2},
        {"1 + 2", 3},
        {"1 + 2 + 1", 4},
        {"1 - 2", -1},
        {"1 * 2", 2},
        {"4 / 2", 2},
        {"50 / 2 * 2 + 10 - 5", 55},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 * 2 * 2 * 2", 32},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"5 * (2 + 10)", 60},
        {"-5", -5},
        {"-10", -10},
        {"-50 + 100 + -50", 0},
        {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, BooleanExpressionTest) {
    VMTestCase tests[] = {
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
        {"!true", false},
        {"!false", true},
        {"!5", false},
        {"!!true", true},
        {"!!false", false},
        {"!!5", true},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, StringExpressionTest) {
    VMTestCase tests[] = {
        {"\"monkey\"", "monkey"},
        {"\"mon\" + \"key\";", "monkey"},
        {"\"mon\" + \"key\" + \"banana\";", "monkeybanana"},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, ArrayLiteralTest) {
    VMTestCase tests[] = {
        {"[]", "[]"},
        {"[1, 2, 3]", "[1, 2, 3]"},
        {"[1+2, 3*4, 5+6]", "[3, 12, 11]"},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, HashLiteralTest) {
    VMTestCase tests[] = {
        {"{}", "{}"},
        {"{1: 2, 2: 3}", "{1: 2, 2: 3}"},
        {"{1 + 1: 2 * 2, 3 + 3: 4 * 4}", "{2: 4, 6: 16}"},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, ConditionTest) {
    VMTestCase tests[] = {
        {"if(true) { 10 }", 10},
        {"if(true) { 10 } else { 20 }", 10},
        {"if(false) { 10 } else { 20 }", 20},
        {"if(1) { 10 }", 10},
        {"if(1 < 2){ 10}", 10},
        {"if(1 < 2){ 10 } else { 20 }", 10},
        {"if(1 > 2){ 10 } else { 20 }", 20},
        {"if( 1 > 2) { 10 }", nullptr},
        {"if( false ){ 10 }", nullptr},
        {"if((if (false) { 10 })){ 10 } else { 20 }", 20},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, GlobalLetStatementTest) {
    VMTestCase tests[] = {
        {"let one = 1; one", 1},
        {"let one = 1; let two = 2; one + two;", 3},
        {"let one = 1; let two = one + one; one + two;", 3},
    };

    RUN_VM_TESTS(tests);
}

}  // namespace monkey
}  // namespace pyc
