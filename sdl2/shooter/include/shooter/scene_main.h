#pragma once

#include <chrono>
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

    void playerProjectileRender();

private:
    Game& game_;
    Player player_;
    Projectile projectile_prototype_;
    std::vector<Projectile> player_projectiles_;
};

}  // namespace sdl2
}  // namespace pyc