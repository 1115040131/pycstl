#pragma once

#include <chrono>
#include <list>
#include <optional>

#include "common/singleton.h"
#include "tetris/piece.h"
#include "tetris/play_field.h"

namespace pyc {
namespace tetris {

class Game : public Singleton<Game> {
    friend class Singleton<Game>;
    friend class Control;

public:
    bool Running() const { return running_; }

    void Init();

    void Process(std::chrono::nanoseconds delta);

    void Render();

    const Matrix& GetPlayField() { return play_field_; }

private:
    /// @brief 随机生成一个块
    Piece GeneratePiece();

    /// @brief 获取一个 piece
    void PickPiece();

    /// @brief 载入预设地图
    void Load();

    void Quit() { running_ = false; }

    void Rotate2() {
        if (!locking_) {
            piece_.Rotate2();
        }
    }

    void RotateR() {
        if (!locking_) {
            piece_.RotateR();
        }
    }

    void RotateL() {
        if (!locking_) {
            piece_.RotateL();
        }
    }

    void Left() {
        if (!locking_) {
            piece_.Left();
        }
    }

    void Right() {
        if (!locking_) {
            piece_.Right();
        }
    }

    void Down() {
        if (!locking_) {
            piece_.Down();
        }
    }

    /// @brief 锁定
    void Lock();

    /// @brief 消行
    void Clear();

    /// @brief 直落
    void Drop();

    /// @brief 暂存
    void Hold();

private:
    static constexpr int kPieceInitX = 4;
    static constexpr int kPieceInitY = 20;
    static constexpr int kPieceInitIndex = 0;

    bool running_{false};
    bool locking_ = false;
    bool holding_ = false;  // 是否使用过暂存

    Matrix play_field_ = std::vector<std::vector<int>>(kPlayFieldRow, std::vector<int>(kPlayFieldCol));
    Piece piece_{};
    std::list<Piece> preview_;         // 预览队列
    std::optional<Piece> hold_piece_;  // 暂存块
};

}  // namespace tetris
}  // namespace pyc
