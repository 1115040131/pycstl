#include <gtest/gtest.h>

#include "monkey/compiler/compiler.h"
#include "monkey/test/test_define.h"

namespace pyc {
namespace monkey {

struct CompilerTestCase {
    std::string input;
    std::vector<std::variant<int>> expected_constants;
    std::vector<Instructions> expected_instructions;
};

Instructions concateInstructions(const std::vector<Instructions>& instructions) {
    Instructions concated;
    for (const auto& instruction : instructions) {
        concated.insert(concated.end(), instruction.begin(), instruction.end());
    }
    return concated;
}

#define TEST_INSTRUCTIONS(expected, actual)            \
    {                                                  \
        auto concated = concateInstructions(expected); \
        ASSERT_EQ(concated.size(), actual.size());     \
        for (size_t i = 0; i < expected.size(); i++) { \
            EXPECT_EQ(concated[i], actual[i]);         \
        }                                              \
    }

#define TEST_CONSTANTS(expected, actual, input)                                    \
    {                                                                              \
        ASSERT_EQ(expected.size(), actual.size());                                 \
        for (size_t i = 0; i < expected.size(); i++) {                             \
            if (std::holds_alternative<int>(expected[i])) {                        \
                TEST_INTEGER_OBJECT(actual[i], std::get<int>(expected[i]), input); \
            }                                                                      \
        }                                                                          \
    }

TEST(CompilerTest, IntegerArithmeticTest) {
    CompilerTestCase tests[] = {
        {
            "1+2",
            {1, 2},
            {
                ByteCode::Make(OpcodeType::OpConstant, {0}),
                ByteCode::Make(OpcodeType::OpConstant, {1}),
                ByteCode::Make(OpcodeType::OpAdd, {}),
            },
        },
    };

    for (const auto& test : tests) {
        auto compiler = Compiler::New();
        auto err = compiler->compile(processInput(test.input));
        ASSERT_FALSE(err);
        TEST_INSTRUCTIONS(test.expected_instructions, compiler->instructions());
        TEST_CONSTANTS(test.expected_constants, compiler->constants(), test.input);
    }
}

}  // namespace monkey
}  // namespace pyc
