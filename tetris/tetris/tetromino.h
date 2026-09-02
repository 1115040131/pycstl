#pragma once

#include <array>
#include <vector>

#include "tetris/ansi.h"

namespace pyc {
namespace tetris {

namespace v1 {

template <std::size_t M, std::size_t N>
using Tetromino = std::array<std::array<char, N>, M>;

inline constexpr Tetromino<5, 5> I{{
    {'0', '0', '0', '0', '0'},
    {'0', '0', '0', '0', '0'},
    {'0', 'I', 'I', 'I', 'I'},
    {'0', '0', '0', '0', '0'},
    {'0', '0', '0', '0', '0'},
}};

inline constexpr Tetromino<3, 3> J{{
    {'J', '0', '0'},
    {'J', 'J', 'J'},
    {'0', '0', '0'},
}};

inline constexpr Tetromino<3, 3> L{{
    {'0', '0', 'L'},
    {'L', 'L', 'L'},
    {'0', '0', '0'},
}};

inline constexpr Tetromino<3, 3> O{{
    {'0', 'O', 'O'},
    {'0', 'O', 'O'},
    {'0', '0', '0'},
}};

inline constexpr Tetromino<3, 3> S{{
    {'0', 'S', 'S'},
    {'S', 'S', '0'},
    {'0', '0', '0'},
}};

inline constexpr Tetromino<3, 3> T{{
    {'0', 'T', '0'},
    {'T', 'T', 'T'},
    {'0', '0', '0'},
}};

inline constexpr Tetromino<3, 3> Z{{
    {'Z', 'Z', '0'},
    {'0', 'Z', 'Z'},
    {'0', '0', '0'},
}};

// 辅助模板，用于在编译期获取数组的旋转
template <std::size_t M, std::size_t N, std::size_t... I>
constexpr Tetromino<N, M> RightRotateImpl(const Tetromino<M, N>& arr, std::index_sequence<I...>) {
    // 通过嵌套的初始化列表构造旋转后的矩阵
    return {{arr[M - 1 - I % M][I / M]...}};
}

/// @brief 右旋(顺时针旋转) 90 度
template <std::size_t M, std::size_t N>
constexpr Tetromino<N, M> RightRotate(const Tetromino<M, N>& arr) {
    // 利用递归模板展开计算旋转矩阵
    return RightRotateImpl<M, N>(arr, std::make_index_sequence<M * N>{});
}

// 辅助模板，用于在编译期获取数组的旋转
template <std::size_t M, std::size_t N, std::size_t... I>
constexpr Tetromino<N, M> LeftRotateImpl(const Tetromino<M, N>& arr, std::index_sequence<I...>) {
    // 通过嵌套的初始化列表构造旋转后的矩阵
    return {{arr[I % M][N - 1 - I / M]...}};
}

/// @brief 作旋(逆时针旋转) 90 度
template <std::size_t M, std::size_t N>
constexpr Tetromino<N, M> LeftRotate(const Tetromino<M, N>& arr) {
    // 利用递归模板展开计算旋转矩阵
    return LeftRotateImpl<M, N>(arr, std::make_index_sequence<M * N>{});
}

}  // namespace v1

namespace v2 {

/*            I
 0000    0010    0000    0100
 1111    0010    0000    0100
 0000    0010    1111    0100
 0000    0010    0000    0100

0x0f00, 0x2222, 0x00f0, 0x4444
*/

struct Tetromino {
    enum class Type { I, J, L, O, S, T, Z };

    Type type;
    ColorId color;
    std::array<int, 4> data;
};

constexpr Tetromino I_set{Tetromino::Type::I, ColorId::kBrightCyan, {0x0f00, 0x2222, 0x00f0, 0x4444}};

}  // namespace v2

inline namespace v3 {

struct Point {
    int x;
    int y;
};

using Tetromino = std::array<Point, 4>;

struct TetrominoSet {
    enum class Type { I, J, L, O, S, T, Z };
    Type type;
    ColorId color;
    std::array<Tetromino, 4> data;
};

namespace prototype {

inline constexpr TetrominoSet I{TetrominoSet::Type::I,
                                ColorId::kBrightCyan,
                                {{
                                    {{{0, 0}, {-1, 0}, {1, 0}, {2, 0}}},   // 0
                                    {{{0, 0}, {0, 1}, {0, -1}, {0, -2}}},  // R
                                    {{{0, 0}, {-2, 0}, {-1, 0}, {1, 0}}},  // 2
                                    {{{0, 0}, {0, 2}, {0, 1}, {0, -1}}},   // L
                                }}};

inline constexpr TetrominoSet J{TetrominoSet::Type::J,
                                ColorId::kBrightBlue,
                                {{
                                    {{{0, 0}, {-1, 1}, {-1, 0}, {1, 0}}},   // 0
                                    {{{0, 0}, {1, 1}, {0, 1}, {0, -1}}},    // R
                                    {{{0, 0}, {-1, 0}, {1, 0}, {1, -1}}},   // 2
                                    {{{0, 0}, {0, 1}, {-1, -1}, {0, -1}}},  // L
                                }}};

inline constexpr TetrominoSet L{TetrominoSet::Type::L,
                                ColorId::kOrange,
                                {{
                                    {{{0, 0}, {-1, 0}, {1, 0}, {1, 1}}},    // 0
                                    {{{0, 0}, {0, 1}, {0, -1}, {1, -1}}},   // R
                                    {{{0, 0}, {-1, -1}, {-1, 0}, {1, 0}}},  // 2
                                    {{{0, 0}, {-1, 1}, {0, 1}, {0, -1}}},   // L
                                }}};

inline constexpr TetrominoSet O{TetrominoSet::Type::O,
                                ColorId::kBrightYellow,
                                {{
                                    {{{0, 0}, {0, 1}, {1, 1}, {1, 0}}},      // 0
                                    {{{0, 0}, {0, -1}, {1, 0}, {1, -1}}},    // R
                                    {{{0, 0}, {-1, -1}, {-1, 0}, {0, -1}}},  // 2
                                    {{{0, 0}, {-1, 1}, {-1, 0}, {0, 1}}},    // L
                                }}};

inline constexpr TetrominoSet S{TetrominoSet::Type::S,
                                ColorId::kGreen,
                                {{
                                    {{{0, 0}, {-1, 0}, {0, 1}, {1, 1}}},    // 0
                                    {{{0, 0}, {0, 1}, {1, 0}, {1, -1}}},    // R
                                    {{{0, 0}, {-1, -1}, {0, -1}, {1, 0}}},  // 2
                                    {{{0, 0}, {-1, 1}, {-1, 0}, {0, -1}}},  // L
                                }}};

inline constexpr TetrominoSet T{TetrominoSet::Type::T,
                                ColorId::kMagenta,
                                {{
                                    {{{0, 0}, {-1, 0}, {0, 1}, {1, 0}}},   // 0
                                    {{{0, 0}, {0, 1}, {1, 0}, {0, -1}}},   // R
                                    {{{0, 0}, {-1, 0}, {1, 0}, {0, -1}}},  // 2
                                    {{{0, 0}, {-1, 0}, {0, 1}, {0, -1}}},  // L
                                }}};

inline constexpr TetrominoSet Z{TetrominoSet::Type::Z,
                                ColorId::kBrightRed,
                                {{
                                    {{{0, 0}, {-1, 1}, {0, 1}, {1, 0}}},    // 0
                                    {{{0, 0}, {1, 1}, {1, 0}, {0, -1}}},    // R
                                    {{{0, 0}, {-1, 0}, {0, -1}, {1, -1}}},  // 2
                                    {{{0, 0}, {-1, -1}, {-1, 0}, {0, 1}}},  // L
                                }}};

}  // namespace prototype

using Offset = std::vector<std::vector<Point>>;

namespace offset {

inline const Offset JLSTZ{
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},      // 0
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},     // R
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},      // 2
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},  // L
};

inline const Offset I{
    {{0, 0}, {-1, 0}, {2, 0}, {-1, 0}, {2, 0}},   // 0
    {{-1, 0}, {0, 0}, {0, 0}, {0, 1}, {0, -2}},   // R
    {{-1, 1}, {1, 1}, {-2, 1}, {1, 0}, {-2, 0}},  // 2
    {{0, 1}, {0, 1}, {0, 1}, {0, -1}, {0, 2}},    // L
};

inline const Offset O{
    {{0, 0}},    // 0
    {{0, -1}},   // R
    {{-1, -1}},  // 2
    {{-1, 0}},   // L
};

}  // namespace offset

}  // namespace v3

}  // namespace tetris
}  // namespace pyc
