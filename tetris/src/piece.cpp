#include "tetris/piece.h"

#include "tetris/game.h"

namespace pyc {
namespace tetris {

bool Piece::Test(int x, int y) const {
    const auto& play_field = Game::GetInstance().GetPlayField();

    for (auto [dx, dy] : tetromino_set.data[index]) {
        // 越界
        if (x + dx < 0 || x + dx >= static_cast<int>(play_field.size()) || y + dy < 0 ||
            y + dy >= static_cast<int>(play_field[0].size())) {
            return false;
        }
        // 有块
        if (play_field[x + dx][y + dy] > 0) {
            return false;
        }
    }
    return true;
}

}  // namespace tetris
}  // namespace pyc
