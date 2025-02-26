#pragma once

#include <memory>

#include <SDL.h>

#include "common/singleton.h"
#include "shooter/scene.h"

namespace pyc {
namespace sdl2 {

class Game : public Singleton<Game> {
    friend class Singleton<Game>;

private:
    Game() = default;

public:
    void run();

    void init();
    void clean();

    SDL_Window* window() const { return window_; }
    SDL_Renderer* renderer() const { return renderer_; }

private:
    void changeScene(std::unique_ptr<Scene> scene);

    void update(std::chrono::duration<double> delta);
    void render();
    void handleEvent(SDL_Event* event);

public:
    static constexpr int kWindowWidth = 600;
    static constexpr int kWindowHeight = 800;
    static constexpr double kFps = 60;

private:
    bool is_running_ = false;
    std::unique_ptr<Scene> current_scene_ = nullptr;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
};

}  // namespace sdl2
}  // namespace pyc