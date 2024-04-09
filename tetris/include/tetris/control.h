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
    static constexpr char kKeyQ = 'q';
    static constexpr char kKeyW = 'w';
    static constexpr char kKeyA = 'a';
    static constexpr char kKeyD = 'd';
    static constexpr char kKeyS = 's';
    static constexpr char kKeySpace = ' ';
};

}  // namespace tetris
}  // namespace pyc
