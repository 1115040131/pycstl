#include <fmt/base.h>

#include "ghost_escape/game.h"

int main() {
    auto& game = pyc::sdl3::Game::GetInstance();
    game.init("Ghost Escape", 1280, 720);
    game.run();
    return 0;
}
