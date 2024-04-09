#pragma once

#include <array>
#include <chrono>

#include "common/singleton.h"
#include "tetris/piece.h"

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

    const PlayField& GetPlayField() { return play_field_; }

private:
    void Quit() { running_ = false; }

    void Rotate() {
        if (!lock_flag_) {
            piece_.Rotate();
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
    void Drop() {
        lock_flag_ = true;
        while (piece_.Down())
            ;
    }

private:
    bool running_{false};
    PlayField play_field_{};
    Piece piece_{};

    bool lock_flag_ = false;
};

}  // namespace tetris
}  // namespace pyc
