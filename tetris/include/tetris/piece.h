#pragma once

#include "tetris/tetromino.h"

namespace pyc {
namespace tetris {

using PlayField = std::array<std::array<int, 22>, 10>;

struct Piece {
    TetrominoSet tetromino_set;
    // 正交直角坐标
    int x;
    int y;
    int index;

    ColorId GetColor() const { return tetromino_set.color; }

    const Tetromino& GetTetromino() const { return tetromino_set.data[index]; }

    bool Test(int x, int y) const;

    void Rotate() { index = (index + 1) % 4; }

    void Left() { Move(-1, 0); }

    void Right() { Move(1, 0); }

    void Down() { Move(0, -1); }

private:
    void Move(int dx, int dy) {
        if (Test(x + dx, y + dy)) {
            x += dx;
            y += dy;
        }
    }
};

}  // namespace tetris
}  // namespace pyc
