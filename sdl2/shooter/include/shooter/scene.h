#pragma once

#include <chrono>

#include <SDL.h>

namespace pyc {
namespace sdl2 {

class Scene {
public:
    virtual ~Scene() = default;

    virtual void update(std::chrono::duration<double> delta) = 0;
    virtual void render() = 0;
    virtual void handleEvent(SDL_Event* event) = 0;

    virtual void init() = 0;
    virtual void clean() = 0;

private:
};

}  // namespace sdl2
}  // namespace pyc