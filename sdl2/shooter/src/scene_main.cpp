#include "shooter/scene_main.h"

#include <algorithm>

#include <SDL_image.h>
#include <fmt/base.h>

#define ASSET_PATH "sdl2/shooter/assets/"
#define ASSET(filename) (ASSET_PATH filename)

namespace pyc {
namespace sdl2 {

void SceneMain::update(std::chrono::duration<double> delta) {
    if (is_player_alive_) {
        keyboardControl(delta);
    }
    playerProjectileUpdate(delta);
    enemyProjectileUpdate(delta);
    spwanEnemy(delta);
    enemyUpdate(delta);
    if (is_player_alive_) {
        playerUpdate(delta);
    }

    // fmt::print("enemy: {} player_projectiles: {} enemy_projectiles: {}\n", enemies_.size(),
    //            player_projectiles_.size(), enemy_projectiles_.size());
}

void SceneMain::render() {
    playerProjectileRender();
    enemyProjectileRender();
    if (is_player_alive_) {
        playerRender();
    }
    enemyRender();
}

void SceneMain::handleEvent(SDL_Event* event) { (void)event; }

void SceneMain::init() {
    // 生成随机数
    std::random_device rd;
    gen_ = std::mt19937(rd());
    dis_ = std::uniform_real_distribution<double>(0, 1);

    // 初始化玩家
    player_.texture = IMG_LoadTexture(game_.renderer(), ASSET("image/SpaceShip.png"));
    SDL_QueryTexture(player_.texture, nullptr, nullptr, &player_.width, &player_.height);
    player_.width /= 4;
    player_.height /= 4;
    player_.position.x = Game::kWindowWidth / 2 - player_.width / 2;
    player_.position.y = Game::kWindowHeight - player_.height;

    // 初始化敌人原型
    enemy_prototype_.texture = IMG_LoadTexture(game_.renderer(), ASSET("image/insect-2.png"));
    SDL_QueryTexture(enemy_prototype_.texture, nullptr, nullptr, &enemy_prototype_.width,
                     &enemy_prototype_.height);
    enemy_prototype_.width /= 4;
    enemy_prototype_.height /= 4;

    // 初始化子弹原型
    player_projectile_prototype_.texture = IMG_LoadTexture(game_.renderer(), ASSET("image/laser-1.png"));
    SDL_QueryTexture(player_projectile_prototype_.texture, nullptr, nullptr, &player_projectile_prototype_.width,
                     &player_projectile_prototype_.height);
    player_projectile_prototype_.width /= 4;
    player_projectile_prototype_.height /= 4;

    enemy_player_projectile_prototype_.texture = IMG_LoadTexture(game_.renderer(), ASSET("image/bullet-1.png"));
    SDL_QueryTexture(enemy_player_projectile_prototype_.texture, nullptr, nullptr,
                     &enemy_player_projectile_prototype_.width, &enemy_player_projectile_prototype_.height);
    enemy_player_projectile_prototype_.width /= 4;
    enemy_player_projectile_prototype_.height /= 4;
}

void SceneMain::clean() {
    enemies_.clear();
    player_projectiles_.clear();
    enemy_projectiles_.clear();

    SDL_DestroyTexture(player_.texture);
    SDL_DestroyTexture(enemy_prototype_.texture);
    SDL_DestroyTexture(player_projectile_prototype_.texture);
    SDL_DestroyTexture(enemy_player_projectile_prototype_.texture);
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
        if (player_.last_fire + player_.cool_down < std::chrono::steady_clock::now()) {
            player_.last_fire = std::chrono::steady_clock::now();
            playerShoot();
        }
    }
}

void SceneMain::playerProjectileUpdate(std::chrono::duration<double> delta) {
    for (auto& projectile : player_projectiles_) {
        projectile.position.y -= projectile.speed * delta.count();
        if (projectile.position.y + projectile.height < 0) {
            projectile.valid = false;
            continue;
        }

        SDL_Rect projectile_rect{
            static_cast<int>(projectile.position.x),
            static_cast<int>(projectile.position.y),
            projectile.width,
            projectile.height,
        };
        for (auto& enemy : enemies_) {
            if (!enemy.valid) {
                continue;
            }

            SDL_Rect enemy_rect{
                static_cast<int>(enemy.position.x),
                static_cast<int>(enemy.position.y),
                enemy.width,
                enemy.height,
            };
            if (SDL_HasIntersection(&projectile_rect, &enemy_rect)) {
                projectile.valid = false;
                enemy.health -= projectile.damage;
                if (enemy.health <= 0) {
                    enemyExplode(enemy);
                    enemy.valid = false;
                }
                break;
            }
        }
    }
    std::erase_if(player_projectiles_, [](const auto& projectile) { return !projectile.valid; });
}

void SceneMain::enemyProjectileUpdate(std::chrono::duration<double> delta) {
    SDL_Rect player_rect{
        static_cast<int>(player_.position.x),
        static_cast<int>(player_.position.y),
        player_.width,
        player_.height,
    };
    for (auto& projectile : enemy_projectiles_) {
        projectile.position.x += projectile.direction.x * projectile.speed * delta.count();
        projectile.position.y += projectile.direction.y * projectile.speed * delta.count();
        if (projectile.position.x + projectile.width < 0 || projectile.position.x > Game::kWindowWidth ||
            projectile.position.y + projectile.height < 0 || projectile.position.y > Game::kWindowHeight) {
            projectile.valid = false;
            continue;
        }

        SDL_Rect projectile_rect{
            static_cast<int>(projectile.position.x),
            static_cast<int>(projectile.position.y),
            projectile.width,
            projectile.height,
        };
        if (is_player_alive_ && SDL_HasIntersection(&player_rect, &projectile_rect)) {
            projectile.valid = false;
            player_.health -= projectile.damage;
        }
    }

    std::erase_if(enemy_projectiles_, [](const auto& projectile) { return !projectile.valid; });
}

void SceneMain::spwanEnemy(std::chrono::duration<double>) {
    if (dis_(gen_) > 1.0 / 60) {
        return;
    }

    auto enemy = enemy_prototype_;
    enemy.position.x = dis_(gen_) * (Game::kWindowWidth - enemy.width);
    enemies_.push_back(std::move(enemy));
}

void SceneMain::enemyUpdate(std::chrono::duration<double> delta) {
    auto now = std::chrono::steady_clock::now();

    SDL_Rect player_rect{
        static_cast<int>(player_.position.x),
        static_cast<int>(player_.position.y),
        player_.width,
        player_.height,
    };

    for (auto& enemy : enemies_) {
        if (!enemy.valid) {
            continue;
        }

        enemy.position.y += enemy.speed * delta.count();
        if (enemy.position.y > Game::kWindowHeight) {
            enemy.valid = false;
            continue;
        }

        if (is_player_alive_) {
            SDL_Rect enemy_rect{
                static_cast<int>(enemy.position.x),
                static_cast<int>(enemy.position.y),
                enemy.width,
                enemy.height,
            };
            if (SDL_HasIntersection(&player_rect, &enemy_rect)) {
                player_.health--;
                enemy.valid = false;
                continue;
            }

            if (enemy.last_fire + enemy.cool_down < now) {
                enemy.last_fire = now;
                enemyShoot(enemy);
            }
        }
    }

    std::erase_if(enemies_, [](const auto& enemy) { return !enemy.valid; });
}

void SceneMain::playerUpdate(std::chrono::duration<double>) {
    if (player_.health <= 0) {
        // TODO: game over
        is_player_alive_ = false;
    }
}

void SceneMain::playerShoot() {
    auto projectile = player_projectile_prototype_;
    projectile.position.x = player_.position.x + player_.width / 2 - projectile.width / 2;
    projectile.position.y = player_.position.y - projectile.height;
    player_projectiles_.push_back(std::move(projectile));
}

void SceneMain::enemyShoot(const Enemy& enemy) {
    auto projectile = enemy_player_projectile_prototype_;
    projectile.position.x = enemy.position.x + enemy.width / 2 - projectile.width / 2;
    projectile.position.y = enemy.position.y + enemy.height;
    projectile.direction = getDirection(
        projectile.position, {player_.position.x + player_.width / 2, player_.position.y + player_.height / 2});
    enemy_projectiles_.push_back(std::move(projectile));
}

void SceneMain::enemyExplode(const Enemy&) {}

void SceneMain::playerRender() {
    SDL_Rect player_rect{
        static_cast<int>(player_.position.x),
        static_cast<int>(player_.position.y),
        player_.width,
        player_.height,
    };
    SDL_RenderCopy(game_.renderer(), player_.texture, nullptr, &player_rect);
}

void SceneMain::enemyRender() {
    for (const auto& enemy : enemies_) {
        SDL_Rect enemy_rect{
            static_cast<int>(enemy.position.x),
            static_cast<int>(enemy.position.y),
            enemy.width,
            enemy.height,
        };
        SDL_RenderCopy(game_.renderer(), enemy.texture, nullptr, &enemy_rect);
    }
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

void SceneMain::enemyProjectileRender() {
    for (const auto& projectile : enemy_projectiles_) {
        SDL_Rect projectile_rect{
            static_cast<int>(projectile.position.x),
            static_cast<int>(projectile.position.y),
            projectile.width,
            projectile.height,
        };
        auto angle = std::atan2(projectile.direction.y, projectile.direction.x) * 180 / M_PI - 90;
        SDL_RenderCopyEx(game_.renderer(), projectile.texture, nullptr, &projectile_rect, angle, nullptr,
                         SDL_FLIP_VERTICAL);
    }
}

SDL_FPoint SceneMain::getDirection(const SDL_FPoint& from, const SDL_FPoint& to) {
    auto dx = to.x - from.x;
    auto dy = to.y - from.y;
    auto length = std::sqrt(dx * dx + dy * dy);
    return {dx / length, dy / length};
}

}  // namespace sdl2
}  // namespace pyc
