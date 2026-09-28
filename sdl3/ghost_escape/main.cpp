#include <memory>

import ghost_escape.core;
import ghost_escape.scene_title;

using namespace pyc::sdl3;

int main() {
    auto& game = Game::GetInstance();
    game.init("Ghost Escape", 1280, 720);
    game.changeScene(std::make_unique<SceneTitle>());
    game.run();
    return 0;
}
