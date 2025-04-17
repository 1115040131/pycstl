#include "ghost_escape/scene_main.h"
#include "ghost_escape/scene_title.h"
#include "sdl3/common/core/game.h"

using namespace pyc::sdl3;

int main() {
    auto& game = Game::GetInstance();
    game.init("Ghost Escape", 1280, 720);
    game.changeScene(std::make_unique<SceneTitle>());
    game.run();
    return 0;
}
