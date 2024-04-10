#include "tetris/game.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <random>
#include <ranges>
#include <vector>

#include "tetris/draw.h"
#include "tetris/utils.h"

namespace pyc {
namespace tetris {

using namespace std::literals::chrono_literals;

Piece PickPiece() {
    // 方块原型
    static constexpr std::array<TetrominoSet, 7> kPrototypes{
        prototype::I, prototype::J, prototype::L, prototype::O, prototype::S, prototype::T, prototype::Z};
    // bag7 算法
    static std::vector<TetrominoSet> bags(kPrototypes.begin(), kPrototypes.end());

    // 随机数生成器初始化
    static std::random_device rd;   // 用于获得种子
    static std::mt19937 gen(rd());  // 使用 Mersenne Twister 算法

    std::uniform_int_distribution<> dis(0, bags.size() - 1);
    int index = dis(gen);
    auto result = std::move(bags[index]);
    bags.erase(bags.begin() + index);
    if (bags.empty()) {
        bags.assign(kPrototypes.begin(), kPrototypes.end());
    }

    return Piece{result, 4, 20, 0, Piece::Type::kNormal};
}

void Game::Init() {
    piece_ = PickPiece();

    // 补充预览队列
    constexpr std::size_t kPreviewSize = 5;
    for (std::size_t i = 0; i < kPreviewSize; i++) {
        preview_.push_back(PickPiece());
    }

    Load();

    running_ = true;
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
                piece_ = std::move(preview_.front());
                preview_.pop_front();
                preview_.push_back(PickPiece());
                lock_flag_ = false;
            } else {
                lock_flag_ = true;
            }
        }
    }
}

void Game::Render() {
    auto frame = play_field_;

    Merge(frame, piece_);

    auto shadow = piece_;
    shadow.type = Piece::Type::kShadow;
    while (shadow.Down())
        ;
    Merge(frame, shadow);

    DrawFrame(frame, 2, 11);

    Matrix preview_field(kPreviewRow, std::vector<int>(kPreviewCol));
    int y = 12;
    for (auto piece : preview_) {
        piece.x = 2;
        piece.y = y;
        Merge(preview_field, piece);
        y -= 3;
    }

    DrawPreview(preview_field, 2, 23);
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
            iter->assign(kPlayFieldCol, 0);
            std::rotate(iter, iter + 1, play_field_.end());
        } else {
            ++iter;
        }
    }
}

void Game::Drop() {
    lock_flag_ = true;
    while (piece_.Down())
        ;
}

void Game::Load() {
    std::ifstream fs("tetris/resource/map.txt");
    assert(fs.is_open());

    std::string line;
    for (auto& row : play_field_ | std::ranges::views::take(20) | std::ranges::views::reverse) {
        std::getline(fs, line);
        for (std::size_t i = 0; i < kPlayFieldRow; i++) {
            if (line[i] == '1') {
                row[i] = static_cast<int>(ColorId::kBrightWhite);
            }
        }
    }
}

}  // namespace tetris
}  // namespace pyc
