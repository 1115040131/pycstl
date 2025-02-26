#include "shooter/scene_main.h"

#include <algorithm>

#include <SDL_image.h>
#include <fmt/base.h>

#define ASSET_PATH "sdl2/shooter/assets/"
#define ASSET(filename) (ASSET_PATH filename)

namespace pyc {
namespace sdl2 {

void SceneMain::update(std::chrono::duration<double> delta) {
    keyboardControl(delta);
    playerProjectileUpdate(delta);
}

void SceneMain::render() {
    playerProjectileRender();

    // 绘制玩家
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

    projectile_prototype_.texture = IMG_LoadTexture(game_.renderer(), ASSET("image/laser-1.png"));
    SDL_QueryTexture(projectile_prototype_.texture, nullptr, nullptr, &projectile_prototype_.width,
                     &projectile_prototype_.height);
    projectile_prototype_.width /= 4;
    projectile_prototype_.height /= 4;
}

void SceneMain::clean() {
    player_projectiles_.clear();

    SDL_DestroyTexture(player_.texture);
    SDL_DestroyTexture(projectile_prototype_.texture);
}

void SceneMain::keyboardControl(std::chrono::duration<double> delta) {
    // 玩家移动
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

    // 玩家射击
    if (key_board_state[SDL_SCANCODE_J]) {
        if (std::chrono::steady_clock::now() - player_.last_fire > player_.cool_down) {
            player_.last_fire = std::chrono::steady_clock::now();
            playerShoot();
        }
    }
}

void SceneMain::playerShoot() {
    auto projectile = projectile_prototype_;
    projectile.position.x = player_.position.x + player_.width / 2 - projectile.width / 2;
    projectile.position.y = player_.position.y - projectile.height;
    player_projectiles_.push_back(std::move(projectile));
}

void SceneMain::playerProjectileUpdate(std::chrono::duration<double> delta) {
    for (auto& projectile : player_projectiles_) {
        projectile.position.y -= projectile.speed * delta.count();
    }

    std::erase_if(player_projectiles_,
                  [](const auto& projectile) { return projectile.position.y + projectile.height < 0; });
}

void SceneMain::playerProjectileRender() {
    for (const auto& projectile : player_projectiles_) {
        SDL_Rect projectile_rect{
            static_cast<int>(projectile.position.x),
            static_cast<int>(projectile.position.y),
            projectile.width,
            projectile.height,
        };
        SDL_RenderCopy(game_.renderer(), projectile.texture, nullptr, &projectile_rect);
    }
}

}  // namespace sdl2
}  // namespace pyc
