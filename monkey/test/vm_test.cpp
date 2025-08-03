#include <gtest/gtest.h>

#include "monkey/test/test_define.h"
#include "monkey/vm/vm.h"

namespace pyc {
namespace monkey {

struct VMTestCase {
    std::string input;
    Expected expected;
};

#define RUN_VM_TESTS(tests)                                                           \
    for (const auto& test : tests) {                                                  \
        auto compiler = Compiler::New();                                              \
        auto err = compiler->compile(processInput(test.input));                       \
        ASSERT_FALSE(err) << "Input: " << test.input << "Error: " << err->inspect();  \
        auto vm = VM::New(compiler);                                                  \
        auto result = vm->run();                                                      \
        if (result) {                                                                 \
            TEST_EXPECTED_OBJECT(result, test.expected, test.input);                  \
        } else {                                                                      \
            TEST_EXPECTED_OBJECT(vm->lastPoppedElement(), test.expected, test.input); \
        }                                                                             \
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

TEST(VMTest, IndexExpressionTest) {
    VMTestCase tests[] = {
        {"[1,2,3][1]", 2},        {"[1,2,3][0 + 2]", 3}, {"[[1,1,1]][0][0]", 1}, {"[][0]", nullptr},
        {"[1,2,3][99]", nullptr}, {"[1][-1]", nullptr},  {"{1: 1, 2:2}[1]", 1},  {"{1: 1, 2:2}[2]", 2},
        {"{1: 1}[0]", nullptr},   {"{}[0]", nullptr},
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

TEST(VMTest, CallingFunctionWithoutArgumentsTest) {
    VMTestCase tests[] = {
        {R""(
                let fivePlusTen = fn(){ 5 + 10; }
                fivePlusTen();
            )"",
         15},
        {
            R""(
                let one = fn(){ 1; };
                let two = fn(){ 2; };
                one() + two();
            )"",
            3},
        {
            R""(
                let a = fn(){ 1; };
                let b = fn(){ a() + 1 };
                let c = fn(){ b() + 1 };
                c();
            )"",
            3},
        {
            R""(
                let earlyExit = fn(){ return 99; 100; };
                earlyExit();
            )"",
            99},
        {
            R""(
                let earlyExit = fn(){ return 99; return 100; }
                earlyExit();
            )"",
            99},
        {
            R""(
                let noReturn = fn() { };
                noReturn();
            )"",
            nullptr},
        {
            R""(
                let noReturn = fn() { };
                let noReturnTwo = fn() { noReturn(); };
                noReturn();
                noReturnTwo();
            )"",
            nullptr},
        {
            R""(
                let returnOne = fn(){ 1; };
                let returnOneReturner = fn(){ returnOne; };
                returnOneReturner()();
            )"",
            1},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, CallingFunctionWithLocalArgumentTest) {
    VMTestCase tests[] = {
        {R""(
                let one = fn(){ let one = 1; one }
                one();
            )"",
         1},
        {R""(
                let oneAndTwo = fn(){ let one = 1; let two = 2; one + two; }
                oneAndTwo();
            )"",
         3},
        {R""(
                let oneAndTwo = fn(){ let one=1; let two=2; one + two; }
                let threeAndFour = fn(){ let three = 3; let four = 4; three + four; }
                oneAndTwo() + threeAndFour();
            )"",
         10},
        {R""(
                let firstFoobar = fn(){ let foobar = 50; foobar; }
                let secondFoobar = fn(){ let foobar = 100; foobar; }
                firstFoobar() + secondFoobar();
            )"",
         150},
        {R""(
                let globalSeed = 50;
                let minusOne = fn(){
                    let num = 1;
                    globalSeed - num;
                }

                let minusTwo = fn(){
                    let num = 2;
                    globalSeed - num;
                }

                minusOne() + minusTwo();
            )"",
         97},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, CallingFirstFunctionTest) {
    VMTestCase tests[] = {
        {R""(
                let returnsOneReturner = fn(){
                    let returnsOne = fn(){ 1 ; };
                    returnsOne;
                }

                returnsOneReturner()();
            )"",
         1},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, CallingFunctionWithArgumentsTest) {
    VMTestCase tests[] = {
        {R""(
                let identify = fn(a) { a; }
                identify(4);
            )"",
         4},
        {R""(
                let sum = fn(a, b){ a + b; }
                sum(1,2);
            )"",
         3},
        {R""(
                let sum = fn(a, b){
                    let c = a + b;
                    c;
                }

                sum(1,2);
            )"",
         3},
        {R""(
                let sum = fn(a, b) {
                    let c = a + b;
                    c;
                }

                sum(1, 2) + sum(3, 4)
            )"",
         10},
        {R""(
                let sum = fn(a, b){
                    let c = a + b;
                    c;
                }

                let outer = fn(){
                    sum(1, 2) + sum(3, 4);
                }

                outer();
            )"",
         10},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, CallingFunctionWithWrongArgumentsTest) {
    VMTestCase tests[] = {
        {R""(
                1 + "123";
            )"",
         "unsupported types for binary operaction: INTEGER OpAdd STRING"},
        {R""(
                1 / 0;
            )"",
         "Division by zero"},
        {R""(
                let a = 2;
                a();
            )"",
         "calling non-function and non-built-in: INTEGER"},
        {R""(
                fn(){ 1; }(1)
            )"",
         "wrong number of arguments: want=0, got=1"},
        {R""(
                fn(a){ a; }()
            )"",
         "wrong number of arguments: want=1, got=0"},
        {R""(
                fn(a, b){ a + b; }(1)
            )"",
         "wrong number of arguments: want=2, got=1"},
    };

    RUN_VM_TESTS(tests);
}

TEST(VMTest, CallingBuiltinFunctionTest) {
    VMTestCase tests[] = {
        {R""(
                len("")
            )"",
         0},
        {R""(
                len("four")
            )"",
         4},
        {R""(
                len("hello world")
            )"",
         11},
        {"last([1, 2, 3])", 3},
        {"len([1, 2, 3])", 3},
        {"first([1, 2, 3])", 1},
        {"len([])", 0},
        {"len(1)", "argument to `len` not supported, got INTEGER"},
        {R""(
                len("one", "two")
            )"",
         "wrong number of arguments. got=2, want=1"},
        {"first(1)", "argument to `first` must be ARRAY, got INTEGER"},
        {"last(1)", "argument to `last` must be ARRAY, got INTEGER"},
        {"push(1, 1)", "argument to `push` must be ARRAY, got INTEGER"},
        {R""(
                puts("hello", "world!")
            )"",
         nullptr},
        {"first([])", nullptr},
        {"last([])", nullptr},
        {"rest([])", nullptr},
        {"rest([1,2,3])", "[2, 3]"},
        {"push([], 1)", "[1]"},
    };

    RUN_VM_TESTS(tests);
}

}  // namespace monkey
}  // namespace pyc
