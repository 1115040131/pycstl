#include "shooter/scene_main.h"

#include <algorithm>

#include <SDL_image.h>
#include <fmt/base.h>



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
    explosionUpdate(delta);
    itemUpdate(delta);

    fmt::println("enemy: {} player_projectiles: {} enemy_projectiles: {} explosions_: {} items_: {}",
                 enemies_.size(), player_projectiles_.size(), enemy_projectiles_.size(), explosions_.size(),
                 items_.size());
}

void SceneMain::render() {
    playerProjectileRender();
    enemyProjectileRender();
    if (is_player_alive_) {
        playerRender();
    }
    enemyRender();
    itemRender();
    explosionRender();
    uiRender();
}

void SceneMain::handleEvent(SDL_Event* event) { (void)event; }

void SceneMain::init() {
    // 载入 bgm
    bgm_ = Mix_LoadMUS(ASSET("music/03_Racing_Through_Asteroids_Loop.ogg"));
    if (!bgm_) {
        fmt::println("Mix_LoadMUS: {}", Mix_GetError());
        return;
    }
    Mix_PlayMusic(bgm_, -1);

    // 读取音效
    sounds_[to_underlying(Sound::kPlayerShoot)] = Mix_LoadWAV(ASSET("sound/laser_shoot4.wav"));
    sounds_[to_underlying(Sound::kEnemyShoot)] = Mix_LoadWAV(ASSET("sound/xs_laser.wav"));
    sounds_[to_underlying(Sound::kPlayerExplode)] = Mix_LoadWAV(ASSET("sound/explosion1.wav"));
    sounds_[to_underlying(Sound::kEnemyExplode)] = Mix_LoadWAV(ASSET("sound/explosion3.wav"));
    sounds_[to_underlying(Sound::kHit)] = Mix_LoadWAV(ASSET("sound/eff11.wav"));
    sounds_[to_underlying(Sound::kGetItem)] = Mix_LoadWAV(ASSET("sound/eff5.wav"));

    // 初始化 UI
    ui_health_ = IMG_LoadTexture(game_.renderer(), ASSET("image/Health UI Black.png"));

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

    // 初始化爆炸原型
    explosion_prototype_.texture = IMG_LoadTexture(game_.renderer(), ASSET("effect/explosion.png"));
    SDL_QueryTexture(explosion_prototype_.texture, nullptr, nullptr, &explosion_prototype_.width,
                     &explosion_prototype_.height);
    explosion_prototype_.total_frame = explosion_prototype_.width / explosion_prototype_.height;
    explosion_prototype_.width = explosion_prototype_.height;

    // 初始化物品原型
    item_life_prototype_.texture = IMG_LoadTexture(game_.renderer(), ASSET("image/bonus_life.png"));
    SDL_QueryTexture(item_life_prototype_.texture, nullptr, nullptr, &item_life_prototype_.width,
                     &item_life_prototype_.height);
    item_life_prototype_.width /= 4;
    item_life_prototype_.height /= 4;
}

void SceneMain::clean() {
    enemies_.clear();
    player_projectiles_.clear();
    enemy_projectiles_.clear();
    explosions_.clear();
    items_.clear();
    for (auto& sound : sounds_) {
        Mix_FreeChunk(sound);
    }
    SDL_DestroyTexture(ui_health_);

    SDL_DestroyTexture(player_.texture);
    SDL_DestroyTexture(enemy_prototype_.texture);
    SDL_DestroyTexture(player_projectile_prototype_.texture);
    SDL_DestroyTexture(enemy_player_projectile_prototype_.texture);
    SDL_DestroyTexture(explosion_prototype_.texture);
    SDL_DestroyTexture(item_life_prototype_.texture);

    Mix_HaltMusic();
    Mix_FreeMusic(bgm_);
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

        auto projectile_rect = getRect(projectile.position, projectile.width, projectile.height);
        for (auto& enemy : enemies_) {
            if (!enemy.valid) {
                continue;
            }

            auto enemy_rect = getRect(enemy.position, enemy.width, enemy.height);
            if (SDL_HasIntersection(&projectile_rect, &enemy_rect)) {
                projectile.valid = false;
                enemy.health -= projectile.damage;
                Mix_PlayChannel(-1, sounds_[to_underlying(Sound::kHit)], 0);
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
    auto player_rect = getRect(player_.position, player_.width, player_.height);
    for (auto& projectile : enemy_projectiles_) {
        projectile.position.x += projectile.direction.x * projectile.speed * delta.count();
        projectile.position.y += projectile.direction.y * projectile.speed * delta.count();
        if (outOfWindow(projectile.position, projectile.width, projectile.height)) {
            projectile.valid = false;
            continue;
        }

        auto projectile_rect = getRect(projectile.position, projectile.width, projectile.height);
        if (is_player_alive_ && SDL_HasIntersection(&player_rect, &projectile_rect)) {
            projectile.valid = false;
            player_.health -= projectile.damage;
            Mix_PlayChannel(-1, sounds_[to_underlying(Sound::kHit)], 0);
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
    auto player_rect = getRect(player_.position, player_.width, player_.height);

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
            auto enemy_rect = getRect(enemy.position, enemy.width, enemy.height);
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

        auto explosion = explosion_prototype_;
        explosion.position = {
            player_.position.x + player_.width / 2 - explosion.width / 2,
            player_.position.y + player_.height / 2 - explosion.height / 2,
        };
        explosion.start = std::chrono::steady_clock::now();
        explosions_.push_back(std::move(explosion));

        Mix_PlayChannel(-1, sounds_[to_underlying(Sound::kPlayerExplode)], 0);
    }
}

void SceneMain::explosionUpdate(std::chrono::duration<double>) {
    auto now = std::chrono::steady_clock::now();
    for (auto& explosion : explosions_) {
        if (explosion.start + explosion.frame_delay * (explosion.current_frame + 1) < now) {
            explosion.current_frame++;
        }
    }

    std::erase_if(explosions_,
                  [](const auto& explosion) { return explosion.current_frame >= explosion.total_frame; });
}

void SceneMain::itemUpdate(std::chrono::duration<double> delta) {
    auto player_rect = getRect(player_.position, player_.width, player_.height);
    for (auto& item : items_) {
        item.position.x += item.direction.x * item.speed * delta.count();
        item.position.y += item.direction.y * item.speed * delta.count();

        if (item.bounce_count > 0) {
            bool bounce = false;
            if (item.position.x < 0 || item.position.x + item.width > Game::kWindowWidth) {
                item.direction.x = -item.direction.x;
                bounce = true;
            }
            if (item.position.y < 0 || item.position.y + item.height > Game::kWindowHeight) {
                item.direction.y = -item.direction.y;
                bounce = true;
            }
            if (bounce) {
                item.bounce_count--;
            }
        } else {
            if (outOfWindow(item.position, item.width, item.height)) {
                item.valid = false;
                continue;
            }
        }

        auto item_rect = getRect(item.position, item.width, item.height);
        if (is_player_alive_ && SDL_HasIntersection(&player_rect, &item_rect)) {
            playerGetItem(item);
            item.valid = false;
        }
    }

    std::erase_if(items_, [](const auto& item) { return !item.valid; });
}

void SceneMain::playerShoot() {
    auto projectile = player_projectile_prototype_;
    projectile.position.x = player_.position.x + player_.width / 2 - projectile.width / 2;
    projectile.position.y = player_.position.y - projectile.height;
    player_projectiles_.push_back(std::move(projectile));

    Mix_PlayChannel(0, sounds_[to_underlying(Sound::kPlayerShoot)], 0);
}

void SceneMain::playerGetItem(const Item& item) {
    switch (item.type) {
        case Item::Type::kLife:
            if (player_.health < player_.max_health) {
                player_.health++;
            }
            break;

        default:
            break;
    }
    Mix_PlayChannel(-1, sounds_[to_underlying(Sound::kGetItem)], 0);
}

void SceneMain::enemyShoot(const Enemy& enemy) {
    auto projectile = enemy_player_projectile_prototype_;
    projectile.position.x = enemy.position.x + enemy.width / 2 - projectile.width / 2;
    projectile.position.y = enemy.position.y + enemy.height;
    projectile.direction = getDirection(
        projectile.position, {player_.position.x + player_.width / 2, player_.position.y + player_.height / 2});
    enemy_projectiles_.push_back(std::move(projectile));

    Mix_PlayChannel(-1, sounds_[to_underlying(Sound::kEnemyShoot)], 0);
}

void SceneMain::enemyExplode(const Enemy& enemy) {
    auto explosion = explosion_prototype_;
    explosion.position = {
        enemy.position.x + enemy.width / 2 - explosion.width / 2,
        enemy.position.y + enemy.height / 2 - explosion.height / 2,
    };
    explosion.start = std::chrono::steady_clock::now();
    explosions_.push_back(std::move(explosion));

    Mix_PlayChannel(-1, sounds_[to_underlying(Sound::kEnemyExplode)], 0);

    if (dis_(gen_) < 0.5) {
        dropItem(enemy);
    }
}

void SceneMain::dropItem(const Enemy& enemy) {
    auto item = item_life_prototype_;
    item.position = {
        enemy.position.x + enemy.width / 2 - item.width / 2,
        enemy.position.y + enemy.height / 2 - item.height / 2,
    };
    auto angle = dis_(gen_) * 2 * M_PI;
    item.direction = {static_cast<float>(std::cos(angle)), static_cast<float>(std::sin(angle))};
    items_.push_back(std::move(item));
}

void SceneMain::playerRender() {
    auto player_rect = getRect(player_.position, player_.width, player_.height);
    SDL_RenderCopy(game_.renderer(), player_.texture, nullptr, &player_rect);
}

void SceneMain::enemyRender() {
    for (const auto& enemy : enemies_) {
        auto enemy_rect = getRect(enemy.position, enemy.width, enemy.height);
        SDL_RenderCopy(game_.renderer(), enemy.texture, nullptr, &enemy_rect);
    }
}

void SceneMain::playerProjectileRender() {
    for (const auto& projectile : player_projectiles_) {
        auto projectile_rect = getRect(projectile.position, projectile.width, projectile.height);
        SDL_RenderCopy(game_.renderer(), projectile.texture, nullptr, &projectile_rect);
    }
}

void SceneMain::enemyProjectileRender() {
    for (const auto& projectile : enemy_projectiles_) {
        auto projectile_rect = getRect(projectile.position, projectile.width, projectile.height);
        auto angle = std::atan2(projectile.direction.y, projectile.direction.x) * 180 / M_PI - 90;
        SDL_RenderCopyEx(game_.renderer(), projectile.texture, nullptr, &projectile_rect, angle, nullptr,
                         SDL_FLIP_VERTICAL);
    }
}

void SceneMain::explosionRender() {
    for (const auto& explosion : explosions_) {
        SDL_Rect src{
            explosion.current_frame * explosion.height,
            0,
            explosion.height,
            explosion.height,
        };
        auto dst = getRect(explosion.position, explosion.width, explosion.height);
        SDL_RenderCopy(game_.renderer(), explosion.texture, &src, &dst);
    }
}

void SceneMain::itemRender() {
    for (const auto& item : items_) {
        auto item_rect = getRect(item.position, item.width, item.height);
        SDL_RenderCopy(game_.renderer(), item.texture, nullptr, &item_rect);
    }
}

void SceneMain::uiRender() {
    int x = 10;
    int y = 10;
    int size = 32;
    int offset = 40;
    SDL_SetTextureColorMod(ui_health_, 255, 255, 255);
    for(int i = 0; i < player_.health; i++) {
        SDL_Rect rect {x + i * offset, y, size, size};
        SDL_RenderCopy(game_.renderer(), ui_health_, nullptr, &rect);
    }
    SDL_SetTextureColorMod(ui_health_, 100, 100, 100);
    for(int i = player_.health; i < player_.max_health; i++) {
        SDL_Rect rect {x + i * offset, y, size, size};
        SDL_RenderCopy(game_.renderer(), ui_health_, nullptr, &rect);
    }
}

SDL_FPoint SceneMain::getDirection(const SDL_FPoint& from, const SDL_FPoint& to) {
    auto dx = to.x - from.x;
    auto dy = to.y - from.y;
    auto length = std::sqrt(dx * dx + dy * dy);
    return {dx / length, dy / length};
}

bool SceneMain::outOfWindow(const SDL_FPoint& position, int width, int height) {
    return position.x + width < 0 || position.x > Game::kWindowWidth || position.y + height < 0 ||
           position.y > Game::kWindowHeight;
}

SDL_Rect SceneMain::getRect(const SDL_FPoint& position, int width, int height) {
    return {
        static_cast<int>(position.x),
        static_cast<int>(position.y),
        width,
        height,
    };
}

}  // namespace sdl2
}  // namespace pyc
