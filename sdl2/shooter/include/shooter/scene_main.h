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
    void keyboardControl(std::chrono::duration<double> delta);
    void playerShoot();
    void playerProjectileUpdate(std::chrono::duration<double> delta);
    void spwanEnemy(std::chrono::duration<double> delta);
    void enemyUpdate(std::chrono::duration<double> delta);

    void enemyRender();
    void playerProjectileRender();

private:
    Game& game_;
    Player player_;

    std::mt19937 gen_;
    std::uniform_real_distribution<double> dis_;

    Enemy enemy_prototype_;
    std::vector<Enemy> enemies_;

    Projectile projectile_prototype_;
    std::vector<Projectile> player_projectiles_;
};

}  // namespace sdl2
}  // namespace pyc