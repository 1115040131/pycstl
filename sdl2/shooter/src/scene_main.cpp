#include "shooter/scene_main.h"

#include <algorithm>

#include <SDL_image.h>

#define ASSET_PATH "sdl2/shooter/assets/"
#define ASSET(filename) (ASSET_PATH filename)

namespace pyc {
namespace sdl2 {

void SceneMain::update(std::chrono::duration<double> delta) { keyboardControl(delta); }

void SceneMain::render() {
    SDL_Rect player_rect{
        static_cast<int>(player_.position.x),
        static_cast<int>(player_.position.y),
        player_.width,
        player_.height,
    };
    SDL_RenderCopy(game_.renderer(), player_.texture, nullptr, &player_rect);
}

void SceneMain::handleEvent(SDL_Event* event) { (void)event; }

void SceneMain::init() {
    player_.texture = IMG_LoadTexture(game_.renderer(), ASSET("image/SpaceShip.png"));
    SDL_QueryTexture(player_.texture, nullptr, nullptr, &player_.width, &player_.height);
    player_.width /= 4;
    player_.height /= 4;
    player_.position.x = Game::kWindowWidth / 2 - player_.width / 2;
    player_.position.y = Game::kWindowHeight - player_.height;
}

void SceneMain::clean() {}

void SceneMain::keyboardControl(std::chrono::duration<double> delta) {
    auto key_board_state = SDL_GetKeyboardState(nullptr);
    if (key_board_state[SDL_SCANCODE_W]) {
        player_.position.y -= player_.speed * delta.count();
    }
    if (key_board_state[SDL_SCANCODE_S]) {
        player_.position.y += player_.speed * delta.count();
    }
    if (key_board_state[SDL_SCANCODE_A]) {
        player_.position.x -= player_.speed * delta.count();
    }
    if (key_board_state[SDL_SCANCODE_D]) {
        player_.position.x += player_.speed * delta.count();
    }

    player_.position.x = std::clamp<float>(player_.position.x, 0, Game::kWindowWidth - player_.width);
    player_.position.y = std::clamp<float>(player_.position.y, 0, Game::kWindowHeight - player_.height);
}

}  // namespace sdl2
}  // namespace pyc