#include "shooter/scene_main.h"

#include <SDL_image.h>

#define ASSET_PATH "sdl2/shooter/assets/"
#define ASSET(filename) (ASSET_PATH filename)

namespace pyc {
namespace sdl2 {

void SceneMain::update() {}

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

}  // namespace sdl2
}  // namespace pyc