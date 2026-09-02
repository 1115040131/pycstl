#include "tetris/utils.h"

namespace pyc {
namespace tetris {

void Merge(Matrix& play_field, const Piece& piece) {
    int x = piece.x;
    int y = piece.y;

    for (auto [dx, dy] : piece.GetTetromino()) {
        if (play_field[y + dy][x + dx] == 0) {
            if (piece.type == Piece::Type::kShadow) {
                play_field[y + dy][x + dx] = -static_cast<int>(piece.GetColor());
            } else {
                play_field[y + dy][x + dx] = static_cast<int>(piece.GetColor());
            }
        }
    }
}

}  // namespace tetris
}  // namespace pyc
