#pragma once

#include <chrono>
#include <memory>
#include <string_view>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "common/singleton.h"
#include "sdl3/common/core/asset_store.h"

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
    const std::unique_ptr<AssetStore>& getAssetStore() const { return asset_store_; }

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

    bool is_running_;

    std::string title_;
    glm::vec2 screen_size_;
    std::shared_ptr<Scene> current_scene_;
    std::unique_ptr<AssetStore> asset_store_;
};

}  // namespace sdl3
}  // namespace pyc
