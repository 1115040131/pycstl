#include "shooter/game.h"

int main() {
    auto& game = pyc::sdl2::Game::GetInstance();
    game.init();
    game.run();
    game.clean();
    return 0;
}
