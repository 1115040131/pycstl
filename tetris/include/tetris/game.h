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

    void Rotate() { piece_.Rotate(); }

    void Left() { piece_.Left(); }

    void Right() { piece_.Right(); }

    void Down() { piece_.Down(); }

private:
    bool running_{false};
    PlayField play_field_{};
    Piece piece_{};
};

}  // namespace tetris
}  // namespace pyc
