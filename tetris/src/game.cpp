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

Piece Game::GeneratePiece() {
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

    return Piece{std::move(result), kPieceInitX, kPieceInitY, kPieceInitIndex, Piece::Type::kNormal};
}

void Game::PickPiece() {
    if (preview_.empty()) {
        piece_ = GeneratePiece();
    } else {
        piece_ = std::move(preview_.front());
        preview_.pop_front();
    }

    // 补充预览队列
    constexpr std::size_t kPreviewSize = 5;
    while (preview_.size() < kPreviewSize) {
        preview_.push_back(GeneratePiece());
    }
}

void Game::Init() {
    Load();
    PickPiece();
    running_ = true;
}

void Game::Load() {
    std::ifstream fs("tetris/resource/map.txt");
    assert(fs.is_open());

    std::string line;
    for (auto& row : play_field_ | std::ranges::views::take(20) | std::ranges::views::reverse) {
        std::getline(fs, line);
        for (std::size_t i = 0; i < kPlayFieldRow; i++) {
            if (line[i] == '1') {
                row[i] = static_cast<int>(ColorId::kBrightBlack);
            }
        }
    }
}

void Game::Process(std::chrono::nanoseconds delta) {
    static std::chrono::nanoseconds lag;
    static constexpr auto kDownDuration = 500ms;
    lag += delta;
    if (lag >= kDownDuration) {
        lag -= kDownDuration;

        // 无法继续下落时, 依然可以调整位置或者旋转), 下一周期再真正锁定
        if (!piece_.Down()) {
            if (locking_) {
                Lock();
                Clear();
                PickPiece();
                locking_ = false;
                holding_ = false;
            } else {
                locking_ = true;
            }
        }
    }
}

void Game::Render() {
    // frame
    auto frame = play_field_;
    Merge(frame, piece_);
    auto shadow = piece_;
    shadow.type = Piece::Type::kShadow;
    while (shadow.Down())
        ;
    Merge(frame, shadow);
    DrawFrame(frame, 2, 11);

    // preview_field
    Matrix preview_field(kPreviewRow, std::vector<int>(kPreviewCol));
    int y = 12;
    for (auto piece : preview_) {
        piece.x = 2;
        piece.y = y;
        Merge(preview_field, piece);
        y -= 3;
    }
    DrawPreview(preview_field, 2, 23);

    // hold_field
    if (hold_piece_.has_value()) {
        Matrix hold_field(kHoldRow, std::vector<int>(kHoldCol, 0));
        auto hold_piece = hold_piece_.value();
        hold_piece.x = 3;
        hold_piece.y = 1;
        hold_piece.index = 0;
        // 不可暂存时将暂存区置白
        if (holding_) {
            hold_piece.tetromino_set.color = ColorId::kBrightWhite;
        }
        Merge(hold_field, hold_piece);
        DrawHold(hold_field, 2, 2);
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
    locking_ = true;
    while (piece_.Down())
        ;
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

}  // namespace tetris
}  // namespace pyc
