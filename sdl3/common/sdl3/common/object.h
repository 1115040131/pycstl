#pragma once

#include <chrono>

#include <SDL3/SDL.h>

#include "sdl3/common/game.h"

namespace pyc {
namespace sdl3 {

class Object {
public:
    Object() = default;
    virtual ~Object() = default;

    virtual void init() {}
    virtual void clean() {}

    virtual void handleEvents(SDL_Event& event) {}
    virtual void update(std::chrono::duration<double> delta) {}
    virtual void render() {}

protected:
    Game& game_ = Game::GetInstance();
};

}  // namespace sdl3
}  // namespace pyc
