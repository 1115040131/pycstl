#pragma once

#include <chrono>
#include <random>
#include <vector>

#include "shooter/game.h"
#include "shooter/object.h"
#include "shooter/scene.h"

namespace pyc {
namespace sdl2 {

class SceneMain : public Scene {
public:
    SceneMain() : game_(Game::GetInstance()) {}

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

    void playerShoot();
    void enemyShoot(const Enemy& enemy);
    void enemyExplode(const Enemy& enemy);

    // render
    void playerRender();
    void enemyRender();
    void playerProjectileRender();
    void enemyProjectileRender();
    void explosionRender();

    // helper
    SDL_FPoint getDirection(const SDL_FPoint& from, const SDL_FPoint& to);

private:
    Game& game_;
    Player player_;
    bool is_player_alive_{true};

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
};

}  // namespace sdl2
}  // namespace pyc