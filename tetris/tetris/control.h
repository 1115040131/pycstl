#pragma once

#include "common/singleton.h"

namespace pyc {
namespace tetris {

class Control {
public:
    static void StartListener();

private:
    static void ProcessCommand();

private:
    static constexpr char KeyQ = 'q';
    static constexpr char KeyW = 'w';
    static constexpr char KeyA = 'a';
    static constexpr char KeyS = 's';
    static constexpr char KeyD = 'd';
    static constexpr char KeyZ = 'z';
    static constexpr char KeyX = 'x';
    static constexpr char KeyC = 'c';
    static constexpr char KeyR = 'r';
    static constexpr char KeyH = 'h';
    static constexpr char KeySpace = ' ';
};

}  // namespace tetris
}  // namespace pyc
