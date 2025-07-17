#include <gtest/gtest.h>

#include "monkey/test/test_define.h"
#include "monkey/vm/vm.h"

namespace pyc {
namespace monkey {

struct VMTestCase {
    std::string input;
    std::variant<int> expected;
};

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
    };

    for (const auto& test : tests) {
        auto compiler = Compiler::New();
        auto err = compiler->compile(processInput(test.input));
        ASSERT_FALSE(err);

        auto vm = VM::New(compiler);
        auto result = vm->run();
        ASSERT_FALSE(result);

        TEST_EXPECTED_OBJECT(vm->lastPoppedElement(), test.expected, test.input);
    }
}

}  // namespace monkey
}  // namespace pyc
