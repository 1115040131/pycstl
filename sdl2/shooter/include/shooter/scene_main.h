#pragma once

#include "shooter/game.h"
#include "shooter/object.h"
#include "shooter/scene.h"

namespace pyc {
namespace sdl2 {

class SceneMain : public Scene {
public:
    SceneMain() : game_(Game::GetInstance()) {}

    virtual ~SceneMain() = default;

    void update() override;
    void render() override;
    void handleEvent(SDL_Event* event) override;

    void init() override;
    void clean() override;

private:
    Game& game_;
    Player player_;
};

}  // namespace sdl2
}  // namespace pyc