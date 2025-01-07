#pragma once

#include <chrono>
#include <memory>

#include "sdl2/common/camera.h"

namespace pyc {
namespace sdl2 {

class Engine {
public:
    void init();
    void mainloop();
    void deinit();

private:
    void load_resources();
    void unload_resources();
    void on_update(std::chrono::duration<double> delta);
    void on_render(const Camera& camera);

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    std::unique_ptr<Camera> camera_;
};

}  // namespace sdl2
}  // namespace pyc