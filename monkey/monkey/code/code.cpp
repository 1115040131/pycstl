#include "monkey/code/code.h"

#include <algorithm>
#include <array>
#include <bit>
#include <map>
#include <numeric>

#include <fmt/format.h>

#include "monkey/macro.h"

namespace pyc {
namespace monkey {

std::string_view toString(OpcodeType type) {
    switch (type) {
        TO_STRING_CASE(OpcodeType, OpConstant);

        TO_STRING_CASE(OpcodeType, OpPop);

        TO_STRING_CASE(OpcodeType, OpAdd);
        TO_STRING_CASE(OpcodeType, OpSub);
        TO_STRING_CASE(OpcodeType, OpMul);
        TO_STRING_CASE(OpcodeType, OpDiv);

        TO_STRING_CASE(OpcodeType, OpTrue);
        TO_STRING_CASE(OpcodeType, OpFalse);
        TO_STRING_CASE(OpcodeType, OpNull);

        TO_STRING_CASE(OpcodeType, OpEqual);
        TO_STRING_CASE(OpcodeType, OpNotEqual);
        TO_STRING_CASE(OpcodeType, OpGreaterThan);

        TO_STRING_CASE(OpcodeType, OpMinus);
        TO_STRING_CASE(OpcodeType, OpBang);

        TO_STRING_CASE(OpcodeType, OpJumpNotTruthy);
        TO_STRING_CASE(OpcodeType, OpJump);

        TO_STRING_CASE(OpcodeType, OpGetGlobal);
        TO_STRING_CASE(OpcodeType, OpSetGlobal);

        TO_STRING_CASE(OpcodeType, OpArray);
        TO_STRING_CASE(OpcodeType, OpHash);
        TO_STRING_CASE(OpcodeType, OpIndex);

        TO_STRING_CASE(OpcodeType, OpCall);
        TO_STRING_CASE(OpcodeType, OpReturnValue);
        TO_STRING_CASE(OpcodeType, OpReturn);
    }
    return "Unknown OpcodeType";
}

// 操作码
struct Definition {
    std::string name;
    std::vector<size_t> operand_widths;
};

inline const std::map<OpcodeType, Definition> definitions{
    {OpcodeType::OpConstant, {"OpConstant", {2}}},

    {OpcodeType::OpPop, {"OpPop", {}}},

    {OpcodeType::OpAdd, {"OpAdd", {}}},
    {OpcodeType::OpSub, {"OpSub", {}}},
    {OpcodeType::OpMul, {"OpMul", {}}},
    {OpcodeType::OpDiv, {"OpDiv", {}}},

    {OpcodeType::OpTrue, {"OpTrue", {}}},
    {OpcodeType::OpFalse, {"OpFalse", {}}},
    {OpcodeType::OpNull, {"OpNull", {}}},

    {OpcodeType::OpEqual, {"OpEqual", {}}},
    {OpcodeType::OpNotEqual, {"OpNotEqual", {}}},
    {OpcodeType::OpGreaterThan, {"OpGreaterThan", {}}},

    {OpcodeType::OpMinus, {"OpMinus", {}}},
    {OpcodeType::OpBang, {"OpBang", {}}},

    {OpcodeType::OpJumpNotTruthy, {"OpJumpNotTruthy", {2}}},
    {OpcodeType::OpJump, {"OpJump", {2}}},

    {OpcodeType::OpGetGlobal, {"OpGetGlobal", {2}}},
    {OpcodeType::OpSetGlobal, {"OpSetGlobal", {2}}},

    {OpcodeType::OpArray, {"OpArray", {2}}},
    {OpcodeType::OpHash, {"OpHash", {2}}},
    {OpcodeType::OpIndex, {"OpIndex", {}}},

    {OpcodeType::OpCall, {"OpCall", {}}},
    {OpcodeType::OpReturnValue, {"OpReturnValue", {}}},
    {OpcodeType::OpReturn, {"OpReturn", {}}},
};

// 整数版本
template <std::integral T>
constexpr T swap_endian(T value) {
#if __cpp_lib_byteswap >= 202110L
    return std::byteswap(value);
#else
    auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(value);
    std::reverse(bytes.begin(), bytes.end());
    return std::bit_cast<T>(bytes);
#endif
}

// 浮点数版本
template <std::floating_point T>
constexpr T swap_endian(T value) {
    // 使用相同大小的整数类型进行转换
    if constexpr (sizeof(T) == 4) {
        auto int_val = std::bit_cast<uint32_t>(value);
        auto swapped = swap_endian(int_val);
        return std::bit_cast<T>(swapped);
    } else if constexpr (sizeof(T) == 8) {
        auto int_val = std::bit_cast<uint64_t>(value);
        auto swapped = swap_endian(int_val);
        return std::bit_cast<T>(swapped);
    } else if constexpr (sizeof(T) == 16) {  // long double可能是16字节
        auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(value);
        std::reverse(bytes.begin(), bytes.end());
        return std::bit_cast<T>(bytes);
    }
}

// 统一的大端序转换函数
template <typename T>
constexpr T to_big_endian(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        return swap_endian(value);
    } else {
        return value;
    }
}

Instructions ByteCode::Make(OpcodeType op, const std::vector<size_t>& operands) {
    auto iter = definitions.find(op);
    if (iter == definitions.end()) {
        return {};
    }
    const auto& definition = iter->second;
    if (definition.operand_widths.size() != operands.size()) {
        fmt::println("op: {} need {} operands, get: {}", toString(op), definition.operand_widths.size(),
                     operands.size());
        return {};
    }

    int instruction_len = std::accumulate(definition.operand_widths.begin(), definition.operand_widths.end(), 1);
    Instructions instructions;
    instructions.reserve(instruction_len);
    instructions.push_back(static_cast<std::underlying_type_t<OpcodeType>>(op));

    for (size_t i = 0; i < operands.size(); i++) {
        auto width = definition.operand_widths[i];
        auto operand = operands[i];
        switch (width) {
            case 1:
                instructions.push_back(static_cast<uint8_t>(operand));
                break;
            case 2: {
                auto bytes = std::bit_cast<std::array<uint8_t, 2>>(to_big_endian(static_cast<uint16_t>(operand)));
                instructions.insert(instructions.end(), bytes.begin(), bytes.end());
            } break;
            case 4: {
                auto bytes = std::bit_cast<std::array<uint8_t, 4>>(to_big_endian(static_cast<uint32_t>(operand)));
                instructions.insert(instructions.end(), bytes.begin(), bytes.end());
            } break;
            default:
                fmt::println("Unsupported operand width: {}", width);
                break;
        }
    }

    return instructions;
}

std::pair<std::vector<size_t>, size_t> ByteCode::ReadOperands(const Instructions& instructions, size_t offset) {
    if (offset >= instructions.size()) {
        fmt::println("Offset {} out of range for instructions size {}", offset, instructions.size());
        return {{}, offset};
    }

    auto iter = definitions.find(static_cast<OpcodeType>(instructions[offset]));
    if (iter == definitions.end()) {
        fmt::println("Unknown opcode: {}", instructions[offset]);
        return {{}, offset + 1};
    }
    const auto& definition = iter->second;
    std::vector<size_t> operands;
    operands.reserve(definition.operand_widths.size());

    offset += 1;  // 跳过操作码
    for (const auto& width : definition.operand_widths) {
        if (offset + width > instructions.size()) {
            fmt::println("Not enough bytes for operand at offset {}, expected width: {}", offset, width);
            return {{}, offset};
        }

        int operand = 0;
        switch (width) {
            case 1:
                operand = instructions[offset];
                break;
            case 2: {
                auto bytes = std::array<uint8_t, 2>{instructions[offset], instructions[offset + 1]};
                operand = to_big_endian(std::bit_cast<uint16_t>(bytes));
            } break;
            case 4: {
                auto bytes = std::array<uint8_t, 4>{instructions[offset], instructions[offset + 1],
                                                    instructions[offset + 2], instructions[offset + 3]};
                operand = to_big_endian(std::bit_cast<uint32_t>(bytes));
            } break;
            default:
                fmt::println("Unsupported operand width: {}", width);
                return {{}, offset};
        }
        offset += width;  // 移动到下一个操作数
        operands.push_back(operand);
    }

    return {operands, offset};
}

std::string toString(const Instructions& instructions) {
    std::string result;
    size_t offset = 0;
    while (offset < instructions.size()) {
        auto opcode = static_cast<OpcodeType>(instructions[offset]);
        result += fmt::format("{:0>4} {}", offset, toString(opcode));
        auto [operands, next_offset] = ByteCode::ReadOperands(instructions, offset);
        for (const auto& operand : operands) {
            result += fmt::format(" {}", operand);
        }
        result += "\n";
        offset = next_offset;
    }
    return result;
}

}  // namespace monkey
}  // namespace pyc