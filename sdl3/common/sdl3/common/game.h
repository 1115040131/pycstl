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

    ~Game() = default;

public:
    SDL_Window* getWindow() const { return window_; }
    SDL_Renderer* getRenderer() const { return renderer_; }
    const std::string& getTitle() const { return title_; }
    const glm::vec2& getScreenSize() const { return screen_size_; }
    const std::shared_ptr<Scene>& getCurrentScene() const { return current_scene_; }

    void init(std::string_view title, int width, int height);
    void run();

    void changeScene(std::unique_ptr<Scene> scene);

    // util
    void drawGrid(const glm::vec2& top_left, const glm::vec2& bottom_right, float grid_width, float grid_height,
                  SDL_FColor color);
    void drawBoundary(const glm::vec2& top_left, const glm::vec2& bottom_right, float boundary_width,
                      SDL_FColor color);

private:
    void clean();

    void handleEvents();
    void update(std::chrono::duration<float> delta);
    void render();

public:
    static constexpr double kFps = 60;

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;

    std::string title_;
    glm::vec2 screen_size_;
    bool is_running_;
    std::shared_ptr<Scene> current_scene_;
};

}  // namespace sdl3
}  // namespace pyc
