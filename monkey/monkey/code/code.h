#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include <fmt/base.h>

namespace pyc {
namespace monkey {

// 字节码
using Opcode = uint8_t;
using Instructions = std::vector<Opcode>;

enum class OpcodeType : Opcode {
    OpConstant,

    OpPop,

    OpAdd,  // '+'
    OpSub,  // '-'
    OpMul,  // '*'
    OpDiv,  // '/'

    OpTrue,   // true
    OpFalse,  // false

    OpEqual,        // ==
    OpNotEqual,     // !=
    OpGreaterThan,  // >

    OpBang,   // !
    OpMinus,  // -
};

std::string_view toString(OpcodeType type);

std::string toString(const Instructions& instructions);

class ByteCode {
public:
    // 生成字节码
    static Instructions Make(OpcodeType op, const std::vector<int>& operands);

    // 读取字节码(反汇编)
    static std::pair<std::vector<int>, size_t> ReadOperands(const Instructions& instructions, size_t offset);
};

}  // namespace monkey
}  // namespace pyc