#include "tetris/game.h"

#include "tetris/draw.h"

namespace pyc {
namespace tetris {

using namespace std::literals::chrono_literals;

Piece PickPiece() { return Piece{prototype::J, 4, 20, 0}; }

void Game::Init() {
    running_ = true;
    piece_ = PickPiece();
}

void Game::Process(std::chrono::nanoseconds delta) {
    static std::chrono::nanoseconds lag;
    static constexpr auto kDownDuration = 500ms;
    lag += delta;
    if (lag >= kDownDuration) {
        lag -= kDownDuration;
        Down();
    }
}

void Game::Render() {
    auto frame = play_field_;

    int x = piece_.x;
    int y = piece_.y;

    // 正常块
    for (auto [dx, dy] : piece_.GetTetromino()) {
        frame[x + dx][y + dy] = static_cast<int>(piece_.GetColor());
    }

    // 阴影块
    while (piece_.Test(x, y - 1)) {
        y--;
    }
    for (auto [dx, dy] : piece_.GetTetromino()) {
        if (frame[x + dx][y + dy] == 0) {
            frame[x + dx][y + dy] = -static_cast<int>(piece_.GetColor());
        }
    }

    DrawFrame(frame, 2, 11);
}

}  // namespace tetris
}  // namespace pyc
