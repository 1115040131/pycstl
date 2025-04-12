#pragma once

#include <chrono>
#include <memory>
#include <random>
#include <string_view>

#include <SDL3/SDL.h>
#include <fmt/base.h>
#include <glm/glm.hpp>

#include "common/singleton.h"
#include "sdl3/common/core/asset_store.h"
#include "sdl3/common/core/texture.h"

namespace pyc {
namespace sdl3 {

class Scene;

class Game : public Singleton<Game> {
    friend class Singleton<Game>;

private:
    Game();

public:
    ~Game();

    SDL_Window* getWindow() const { return window_; }
    SDL_Renderer* getRenderer() const { return renderer_; }

    const std::string& getTitle() const { return title_; }
    const glm::vec2& getScreenSize() const { return screen_size_; }

    const glm::vec2& getMousePosition() const { return mouse_position_; }
    SDL_MouseButtonFlags getMouseButtonFlags() const { return mouse_button_flags_; }

    const std::unique_ptr<Scene>& getCurrentScene() const { return current_scene_; }
    const std::unique_ptr<AssetStore>& getAssetStore() const { return asset_store_; }

    void init(std::string_view title, int width, int height);
    void run();

    void changeScene(std::unique_ptr<Scene> scene);

    // random
    template <typename T>
    T random(T min, T max) {
        static_assert(std::is_arithmetic_v<T>, "Template argument must be an integral or floating point type");

        if constexpr (std::is_integral_v<T>) {
            return std::uniform_int_distribution<T>(min, max)(gen_);
        } else if constexpr (std::is_floating_point_v<T>) {
            return std::uniform_real_distribution<T>(min, max)(gen_);
        }
    }

    template <typename T>
    glm::vec<2, T> random(const glm::vec<2, T>& min, const glm::vec<2, T>& max) {
        return {random(min.x, max.x), random(min.y, max.y)};
    }

    // render
    void renderTexture(const Texture& texture, const glm::vec2& position, const glm::vec2& size) const;
    void renderFillCircle(const glm::vec2& position, const glm::vec2& size, float alpha = 1.F) const;
    void renderHBar(const glm::vec2& position, const glm::vec2& size, float percent, SDL_FColor color) const;

    // util
    void drawGrid(const glm::vec2& top_left, const glm::vec2& bottom_right, float grid_width, float grid_height,
                  SDL_FColor color) const;
    void drawBoundary(const glm::vec2& top_left, const glm::vec2& bottom_right, float boundary_width,
                      SDL_FColor color) const;

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

    glm::vec2 mouse_position_;
    SDL_MouseButtonFlags mouse_button_flags_;

    std::unique_ptr<Scene> current_scene_;
    std::unique_ptr<AssetStore> asset_store_;

    // 随机数生成器
    std::mt19937 gen_ = std::mt19937(std::random_device{}());
};

}  // namespace sdl3
}  // namespace pyc
