#pragma once

#include <chrono>
#include <list>

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
    void Quit() { running_ = false; }

    void Rotate2() {
        if (!lock_flag_) {
            piece_.Rotate2();
        }
    }

    void RotateR() {
        if (!lock_flag_) {
            piece_.RotateR();
        }
    }

    void RotateL() {
        if (!lock_flag_) {
            piece_.RotateL();
        }
    }

    void Left() {
        if (!lock_flag_) {
            piece_.Left();
        }
    }

    void Right() {
        if (!lock_flag_) {
            piece_.Right();
        }
    }

    void Down() {
        if (!lock_flag_) {
            piece_.Down();
        }
    }

    /// @brief 锁定
    void Lock();

    /// @brief 消行
    void Clear();

    /// @brief 直落
    void Drop();

    /// @brief 载入预设地图
    void Load();

private:
    bool running_{false};
    Matrix play_field_ = std::vector<std::vector<int>>(kPlayFieldRow, std::vector<int>(kPlayFieldCol));
    Piece piece_{};
    std::list<Piece> preview_;  // 预览队列

    bool lock_flag_ = false;
};

}  // namespace tetris
}  // namespace pyc
