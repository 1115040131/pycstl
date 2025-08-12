#include "tetris/game.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <random>
#include <ranges>
#include <vector>

#include "tetris/draw.h"
#include "tetris/utils.h"

namespace pyc {
namespace tetris {

using namespace std::literals::chrono_literals;

Piece Game::GeneratePiece() {
    // 方块原型
    static constexpr std::array<TetrominoSet, 7> kPrototypes{
        prototype::I, prototype::J, prototype::L, prototype::O, prototype::S, prototype::T, prototype::Z};

    // bag7 算法
    if (bags_.empty()) {
        bags_.assign(kPrototypes.begin(), kPrototypes.end());
    }

    // 随机数生成器初始化
    static std::random_device rd;   // 用于获得种子
    static std::mt19937 gen(rd());  // 使用 Mersenne Twister 算法
    std::uniform_int_distribution<> dis(0, bags_.size() - 1);

    int index = dis(gen);
    auto result = std::move(bags_[index]);
    bags_.erase(bags_.begin() + index);

    return Piece{std::move(result), kPieceInitX, kPieceInitY, kPieceInitIndex, Piece::Type::kNormal};
}

void Game::PickPiece() {
    if (preview_.empty()) {
        piece_ = GeneratePiece();
    } else {
        piece_ = std::move(preview_.front());
        preview_.pop_front();
    }

    if (!piece_.Test(kPieceInitX, kPieceInitY, kPieceInitIndex)) {
        ending_ = true;
    }

    // 补充预览队列
    constexpr std::size_t kPreviewSize = 5;
    while (preview_.size() < kPreviewSize) {
        preview_.push_back(GeneratePiece());
    }
}

void Game::Init() {
    running_ = true;
    locking_ = false;
    holding_ = false;
    ending_ = false;
    level_ = 1;
    score_ = 0;
    lines_ = 0;
    down_duration_ = 800ms;

    play_field_ = std::vector<std::vector<int>>(kPlayFieldRow, std::vector<int>(kPlayFieldCol));
    bags_.clear();
    preview_.clear();
    hold_piece_.reset();

    Load();
    PickPiece();
}

void Game::Load() {
    std::ifstream fs("tetris/resource/map.txt");
    assert(fs.is_open());

    std::string line;
    for (auto& row : play_field_ | std::ranges::views::take(20) | std::ranges::views::reverse) {
        std::getline(fs, line);
        for (std::size_t i = 0; i < kPlayFieldCol; i++) {
            if (line[i] == '1') {
                row[i] = static_cast<int>(ColorId::kBrightBlack);
            } else {
                row[i] = 0;
            }
        }
    }
}

void Game::Process(std::chrono::nanoseconds delta) {
    if (ending_) {
        return;
    }
    UpdateFps(delta);
    UpdateLogic(delta);
    UpdateRender();
}

void Game::UpdateFps(std::chrono::nanoseconds delta) {
    static std::chrono::nanoseconds lag{};
    static std::size_t frame_count{};
    lag += delta;
    frame_count++;

    if (lag >= 1s) {
        fps_ = frame_count / std::chrono::duration_cast<std::chrono::seconds>(lag).count();
        lag -= std::chrono::duration_cast<std::chrono::seconds>(lag);
        frame_count = 0;
    }
}

void Game::UpdateLogic(std::chrono::nanoseconds delta) {
    static std::chrono::nanoseconds lag;
    lag += delta;
    if (lag >= down_duration_) {
        lag -= down_duration_;

        // 无法继续下落时, 依然可以调整位置或者旋转), 下一周期再真正锁定
        if (!piece_.Down()) {
            if (locking_) {
                Lock();
                Clear();
                Levelup();
                PickPiece();
                locking_ = false;
                holding_ = false;
            } else {
                locking_ = true;
                score_++;  // Soft Drop
            }
        }
    }
}

void Game::UpdateRender() {
    // frame
    frame_ = play_field_;
    Merge(frame_, piece_);
    auto shadow = piece_;
    shadow.type = Piece::Type::kShadow;
    while (shadow.Down());
    Merge(frame_, shadow);

    // preview_field
    preview_field_ = std::vector<std::vector<int>>(kPreviewRow, std::vector<int>(kPreviewCol));
    int y = 12;
    for (auto piece : preview_) {
        piece.x = 2;
        piece.y = y;
        Merge(preview_field_, piece);
        y -= 3;
    }

    // hold_field
    hold_field_ = std::vector<std::vector<int>>(kHoldRow, std::vector<int>(kHoldCol, 0));
    if (hold_piece_.has_value()) {
        auto hold_piece = hold_piece_.value();
        hold_piece.x = 3;
        hold_piece.y = 1;
        hold_piece.index = 0;
        // 不可暂存时将暂存区置白
        if (holding_) {
            hold_piece.tetromino_set.color = ColorId::kBrightWhite;
        }
        Merge(hold_field_, hold_piece);
    }
}

void Game::Lock() {
    int x = piece_.x;
    int y = piece_.y;
    for (auto [dx, dy] : piece_.GetTetromino()) {
        play_field_[y + dy][x + dx] = static_cast<int>(piece_.GetColor());
    }
}

void Game::Clear() {
    int count = 0;  // 消除行数
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
            count++;
        } else {
            ++iter;
        }
    }

    if (count == 1) {
        score_ += 100 * level_;
    } else if (count == 2) {
        score_ += 300 * level_;
    } else if (count == 3) {
        score_ += 500 * level_;
    } else if (count == 4) {
        score_ += 800 * level_;
    }
    lines_ += count;
}

void Game::Drop() {
    if (!locking_) {
        locking_ = true;
        while (piece_.Down());
        score_ += 2;  // Hard Drop
    }
}

// 1. 如果暂存区为空, 当前块放入暂存区, 重新生成一个块从头掉落
// 2. 如果暂存区不为空, 当前块和暂存区块交换, 从头掉落
// 3. 一回合只能使用一次暂存
void Game::Hold() {
    if (!holding_) {
        holding_ = true;
        if (!hold_piece_.has_value()) {
            hold_piece_ = std::move(piece_);
            PickPiece();
        } else {
            std::swap(hold_piece_.value(), piece_);
            piece_.x = kPieceInitX;
            piece_.y = kPieceInitY;
        }
    }
}

void Game::Levelup() {
    level_ = lines_ / 10 + 1;
    down_duration_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(std::pow((0.8 - (level_ - 1) * 0.007), level_ - 1)));
}

void Game::Reset() { Init(); }

void Game::Help() { helping_ = !helping_; }

}  // namespace tetris
}  // namespace pyc
