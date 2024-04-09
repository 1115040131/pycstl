#include "tetris/piece.h"

#include "tetris/game.h"

namespace pyc {
namespace tetris {

bool Piece::Test(int target_x, int target_y, int target_index) const {
    const auto& play_field = Game::GetInstance().GetPlayField();

    for (auto [dx, dy] : tetromino_set.data[target_index]) {
        // 越界
        if ((target_x + dx < 0 || target_x + dx >= static_cast<int>(kPlayFieldCol)) ||
            (target_y + dy < 0 || target_y + dy >= static_cast<int>(kPlayFieldRow))) {
            return false;
        }
        // 有块
        if (play_field[target_y + dy][target_x + dx] > 0) {
            return false;
        }
    }
    return true;
}

bool Piece::Move(int dx, int dy, int target_index) {
    if (Test(x + dx, y + dy, target_index)) {
        x += dx;
        y += dy;
        index = target_index;
        return true;
    }
    return false;
}

}  // namespace tetris
}  // namespace pyc
