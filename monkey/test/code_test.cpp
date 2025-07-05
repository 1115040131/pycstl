
#include <gtest/gtest.h>

#include "monkey/code/code.h"

namespace pyc {
namespace monkey {

TEST(CodeTest, MakerTest) {
    struct Input {
        OpcodeType op;
        std::vector<int> operands;
        Instructions expected;
    };

    Input inputs[] = {
        {OpcodeType::OpConstant,
         {65534},
         {static_cast<std::underlying_type_t<OpcodeType>>(OpcodeType::OpConstant), 0xFF, 0xFE}},
    };

    for (const auto& input : inputs) {
        auto instructions = ByteCode::Make(input.op, input.operands);
        EXPECT_EQ(instructions.size(), input.expected.size());
        for (size_t i = 0; i < instructions.size(); i++) {
            EXPECT_EQ(instructions[i], input.expected[i]);
        }
    }
}

TEST(CodeTest, ReadOperandsTest) {
    struct Input {
        OpcodeType op;
        std::vector<int> operands;
        size_t byte_read;
    };

    Input inputs[] = {
        {OpcodeType::OpConstant, {65534}, 3},
    };

    size_t offset = 0;
    for (const auto& input : inputs) {
        auto instructions = ByteCode::Make(input.op, input.operands);
        auto [operands, next_offset] = ByteCode::ReadOperands(instructions, offset);
        EXPECT_EQ(next_offset - offset, input.byte_read);

        for (size_t i = 0; i < input.operands.size(); i++) {
            EXPECT_EQ(operands[i], input.operands[i]);
        }
    }
}

TEST(CodeTest, InstructionsToStringTest) {
    std::vector<Instructions> instructions{
        ByteCode::Make(OpcodeType::OpConstant, {1}),
        ByteCode::Make(OpcodeType::OpConstant, {2}),
        ByteCode::Make(OpcodeType::OpConstant, {65534}),
        ByteCode::Make(OpcodeType::OpConstant, {65535}),
    };

    std::string expected = R"(0000 OpConstant 1
0003 OpConstant 2
0006 OpConstant 65534
0009 OpConstant 65535
)";

    Instructions concated;
    for (const auto& instr : instructions) {
        concated.insert(concated.end(), instr.begin(), instr.end());
    }

    EXPECT_STREQ(toString(concated).c_str(), expected.c_str());
}

}  // namespace monkey
}  // namespace pyc
