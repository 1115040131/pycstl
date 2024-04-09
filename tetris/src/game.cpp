#include "tetris/game.h"

#include <algorithm>
#include <random>
#include <vector>

#include "tetris/draw.h"

namespace pyc {
namespace tetris {

using namespace std::literals::chrono_literals;

Piece PickPiece() {
    static constexpr std::array<TetrominoSet, 7> prototypes{prototype::I, prototype::J, prototype::L, prototype::O,
                                                            prototype::S, prototype::T, prototype::Z};

    // 随机数生成器初始化
    std::random_device rd;   // 用于获得种子
    std::mt19937 gen(rd());  // 使用 Mersenne Twister 算法

    std::uniform_int_distribution<> dis(0, prototypes.size() - 1);

    return Piece{prototypes[dis(gen)], 4, 20, 0};
}

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
        if (!piece_.Down()) {
            if (lock_flag_) {
                Lock();
                Clear();
                piece_ = PickPiece();
                lock_flag_ = false;
            } else {
                lock_flag_ = true;
            }
        }
    }
}

void Game::Render() {
    auto frame = play_field_;

    int x = piece_.x;
    int y = piece_.y;

    // 正常块
    for (auto [dx, dy] : piece_.GetTetromino()) {
        frame[y + dy][x + dx] = static_cast<int>(piece_.GetColor());
    }

    // 阴影块
    while (piece_.Test(x, y - 1, piece_.index)) {
        y--;
    }
    for (auto [dx, dy] : piece_.GetTetromino()) {
        if (frame[y + dy][x + dx] == 0) {
            frame[y + dy][x + dx] = -static_cast<int>(piece_.GetColor());
        }
    }

    DrawFrame(frame, 2, 11);
}

void Game::Lock() {
    int x = piece_.x;
    int y = piece_.y;
    for (auto [dx, dy] : piece_.GetTetromino()) {
        play_field_[y + dy][x + dx] = static_cast<int>(piece_.GetColor());
    }
}

void Game::Clear() {
    for (auto iter = play_field_.begin(); iter != play_field_.end();) {
        bool full = true;
        for (int cell : *iter) {
            if (cell == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            std::rotate(iter, iter + 1, play_field_.end());
            play_field_.rbegin()->fill(0);
        } else {
            ++iter;
        }
    }
}

}  // namespace tetris
}  // namespace pyc
