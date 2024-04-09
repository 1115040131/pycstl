#pragma once

#include "tetris/tetromino.h"

namespace pyc {
namespace tetris {

inline constexpr std::size_t kPlayFieldRow = 22;
inline constexpr std::size_t kPlayFieldCol = 10;

using PlayField = std::array<std::array<int, kPlayFieldCol>, kPlayFieldRow>;

struct Piece {
    TetrominoSet tetromino_set;
    // 正交直角坐标
    int x;
    int y;
    int index;

    ColorId GetColor() const { return tetromino_set.color; }

    const Tetromino& GetTetromino() const { return tetromino_set.data[index]; }

    bool Test(int target_x, int target_y, int target_index) const;

    bool Rotate() { return Move(0, 0, (index + 1) % 4); }

    bool Left() { return Move(-1, 0, index); }

    bool Right() { return Move(1, 0, index); }

    bool Down() { return Move(0, -1, index); }

private:
    bool Move(int dx, int dy, int target_index);
};

}  // namespace tetris
}  // namespace pyc
