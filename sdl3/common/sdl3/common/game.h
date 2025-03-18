#pragma once

#include <chrono>
#include <memory>
#include <string_view>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "common/singleton.h"

namespace pyc {
namespace sdl3 {

class Scene;

class Game : public Singleton<Game> {
    friend class Singleton<Game>;

private:
    Game() = default;

public:
    void init(std::string_view title, int width, int height);
    void run();

    void changeScene(std::unique_ptr<Scene> scene);

private:
    void clean();

    void handleEvents();
    void update(std::chrono::duration<double> delta);
    void render();

public:
    static constexpr double kFps = 60;

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;

    std::string title_;
    glm::vec2 screen_size_;
    bool is_running_;
    std::unique_ptr<Scene> current_scene_;
};

}  // namespace sdl3
}  // namespace pyc
