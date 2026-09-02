#pragma once

#include <array>
#include <chrono>
#include <random>
#include <vector>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "shooter/object.h"
#include "shooter/scene.h"

namespace pyc {
namespace sdl2 {

class SceneMain : public Scene {
public:
    virtual ~SceneMain() = default;

    void update(std::chrono::duration<double> delta) override;
    void render() override;
    void handleEvent(SDL_Event* event) override;

    void init() override;
    void clean() override;

private:
    // update
    void keyboardControl(std::chrono::duration<double> delta);
    void playerProjectileUpdate(std::chrono::duration<double> delta);
    void enemyProjectileUpdate(std::chrono::duration<double> delta);
    void spwanEnemy(std::chrono::duration<double> delta);
    void enemyUpdate(std::chrono::duration<double> delta);
    void playerUpdate(std::chrono::duration<double> delta);
    void explosionUpdate(std::chrono::duration<double> delta);
    void itemUpdate(std::chrono::duration<double> delta);
    void changeSceneDelay(std::chrono::duration<double> delta);

    void playerShoot();
    void playerGetItem(const Item& item);
    void enemyShoot(const Enemy& enemy);
    void enemyExplode(const Enemy& enemy);
    void dropItem(const Enemy& enemy);

    // render
    void playerRender();
    void enemyRender();
    void playerProjectileRender();
    void enemyProjectileRender();
    void explosionRender();
    void itemRender();
    void uiRender();

    // helper
    SDL_FPoint getDirection(const SDL_FPoint& from, const SDL_FPoint& to);
    bool outOfWindow(const SDL_FPoint& position, int width, int height);
    SDL_Rect getRect(const SDL_FPoint& position, int width, int height);

private:
    Player player_;
    bool is_player_alive_{true};
    int score_{};
    Mix_Music* bgm_{};
    SDL_Texture* ui_health_{};
    TTF_Font* score_font_{};
    std::chrono::duration<double> time_end_{};

    std::mt19937 gen_;
    std::uniform_real_distribution<double> dis_;

    Enemy enemy_prototype_;
    std::vector<Enemy> enemies_;

    Projectile player_projectile_prototype_;
    std::vector<Projectile> player_projectiles_;

    Projectile enemy_player_projectile_prototype_;
    std::vector<Projectile> enemy_projectiles_;

    Explosion explosion_prototype_;
    std::vector<Explosion> explosions_;

    Item item_life_prototype_;
    std::vector<Item> items_;

    enum class Sound : size_t {
        kPlayerShoot,
        kEnemyShoot,
        kPlayerExplode,
        kEnemyExplode,
        kHit,
        kGetItem,

        kCount,
    };
    std::array<Mix_Chunk*, to_underlying(Sound::kCount)> sounds_;
};

}  // namespace sdl2
}  // namespace pyc