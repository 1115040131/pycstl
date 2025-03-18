#pragma once

#include <chrono>
#include <string_view>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "common/singleton.h"

namespace pyc {
namespace sdl3 {

class Game : public Singleton<Game> {
    friend class Singleton<Game>;

private:
    Game() = default;

public:
    void init(std::string_view title, int width, int height);
    void run();

private:
    void clean();

    void handleEvents();
    void update(std::chrono::duration<double> delta);
    void render();

public:
    static constexpr double kFps = 60;

private:
    std::string title_;
    glm::vec2 screen_size_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;

    bool is_running_;
};

}  // namespace sdl3
}  // namespace pyc
