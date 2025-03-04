#pragma once

#include <chrono>

#include <SDL.h>

namespace pyc {
namespace sdl2 {

class Game;

class Scene {
public:
    Scene();

    virtual ~Scene() = default;

    virtual void update(std::chrono::duration<double> delta) = 0;
    virtual void render() = 0;
    virtual void handleEvent(SDL_Event* event) = 0;

    virtual void init() = 0;
    virtual void clean() = 0;

protected:
    Game& game_;
};

}  // namespace sdl2
}  // namespace pyc