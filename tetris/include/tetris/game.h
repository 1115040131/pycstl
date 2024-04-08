#pragma once

#include "common/singleton.h"
#include "tetris/tetromino.h"

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

    int Index() const { return index_; }

    Tetromino Curr() const { return curr_; }

private:
    void Quit() { running_ = false; }

    void Rotate() { index_ = (index_ + 1) % 4; }

    void Left() { col_--; }

    void Right() { col_++; }

    void Down() { row_++; }

private:
    bool running_{false};
    int row_{2};
    int col_{15};
    int index_{0};
    Tetromino curr_{I};
};

}  // namespace tetris
}  // namespace pyc
