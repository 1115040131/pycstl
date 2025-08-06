
#include <gtest/gtest.h>

#include "monkey/test/test_define.h"

namespace pyc {
namespace monkey {

TEST(CodeTest, MakerTest) {
    struct Input {
        OpcodeType op;
        std::vector<size_t> operands;
        Instructions expected;
    };

    Input inputs[] = {
        {
            OpcodeType::OpConstant,
            {65534},
            {static_cast<std::underlying_type_t<OpcodeType>>(OpcodeType::OpConstant), 0xFF, 0xFE},
        },
        {
            OpcodeType::OpAdd,
            {},
            {static_cast<std::underlying_type_t<OpcodeType>>(OpcodeType::OpAdd)},
        },
        {
            OpcodeType::OpGetLocal,
            {255},
            {static_cast<std::underlying_type_t<OpcodeType>>(OpcodeType::OpGetLocal), 0xFF},
        },
        {
            OpcodeType::OpClosure,
            {65534, 255},
            {static_cast<std::underlying_type_t<OpcodeType>>(OpcodeType::OpClosure), 0xFF, 0xFE, 0xFF},
        },
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
        std::vector<size_t> operands;
        size_t byte_read;
    };

    Input inputs[] = {
        {OpcodeType::OpConstant, {65534}, 3},
        {OpcodeType::OpAdd, {}, 1},
        {OpcodeType::OpGetLocal, {255}, 2},
        {OpcodeType::OpClosure, {65535, 255}, 4},
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
    struct Input {
        std::vector<Instructions> instructions;
        std::string expected;
    };

    Input inputs[] = {
        {{
             ByteCode::Make(OpcodeType::OpAdd, {}),
             ByteCode::Make(OpcodeType::OpConstant, {2}),
             ByteCode::Make(OpcodeType::OpConstant, {65534}),
             ByteCode::Make(OpcodeType::OpConstant, {65535}),
         },
         R"(0000 OpAdd
0001 OpConstant 2
0004 OpConstant 65534
0007 OpConstant 65535
)"},
        {{
             ByteCode::Make(OpcodeType::OpAdd, {}),
             ByteCode::Make(OpcodeType::OpGetLocal, {1}),
             ByteCode::Make(OpcodeType::OpConstant, {2}),
             ByteCode::Make(OpcodeType::OpConstant, {65535}),
         },
         R""(0000 OpAdd
0001 OpGetLocal 1
0003 OpConstant 2
0006 OpConstant 65535
)""},
        {{
             ByteCode::Make(OpcodeType::OpAdd, {}),
             ByteCode::Make(OpcodeType::OpGetLocal, {1}),
             ByteCode::Make(OpcodeType::OpConstant, {2}),
             ByteCode::Make(OpcodeType::OpConstant, {65535}),
             ByteCode::Make(OpcodeType::OpClosure, {65535, 255}),
         },
         R""(0000 OpAdd
0001 OpGetLocal 1
0003 OpConstant 2
0006 OpConstant 65535
0009 OpClosure 65535 255
)""},
    };

    for (const auto& input : inputs) {
        EXPECT_EQ(toString(concateInstructions(input.instructions)), input.expected);
    }
}

}  // namespace monkey
}  // namespace pyc
