#pragma once

#include "common/singleton.h"

namespace pyc {
namespace tetris {

class Game : public Singleton<Game> {
    friend class Singleton<Game>;
    friend class Control;

public:
    bool Running() const { return running_; }

    void Init() { running_ = true; }

    int Row() const { return row_; }

    int Col() const { return col_; }

private:
    void Quit() { running_ = false; }

    void Rotate() { row_--; }

    void Left() { col_--; }

    void Right() { col_++; }

    void Down() { row_++; }

private:
    bool running_{false};
    int row_{2};
    int col_{15};
};

}  // namespace tetris
}  // namespace pyc
