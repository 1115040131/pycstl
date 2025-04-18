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

    const std::string& getTitle() const { return title_; }
    const glm::vec2& getScreenSize() const { return screen_size_; }

    const glm::vec2& getMousePosition() const { return mouse_position_; }
    SDL_MouseButtonFlags getMouseButtonFlags() const { return mouse_button_flags_; }

    const std::unique_ptr<Scene>& getCurrentScene() const { return current_scene_; }
    const std::unique_ptr<AssetStore>& getAssetStore() const { return asset_store_; }

    void init(std::string_view title, int width, int height);
    void run();

    void changeScene(std::unique_ptr<Scene> scene);

    void setScore(int score);
    void addScore(int score);
    int getScore() const { return score_; }
    int getHighScore() const { return high_score_; }

    void quit() { is_running_ = false; }

    // audio
    void playMusic(const std::string& file_path, bool loop = true) const {
        Mix_PlayMusic(asset_store_->getMusic(file_path), loop ? -1 : 1);
    }
    void playSound(const std::string& file_path) const {
        Mix_PlayChannel(-1, asset_store_->getSound(file_path), 0);
    }
    void stopMusic() const { Mix_HaltMusic(); }
    void stopSound() const { Mix_HaltChannel(-1); }
    void pauseMusic() const { Mix_PauseMusic(); }
    void pauseSound() const { Mix_Pause(-1); }
    void resumeMusic() const { Mix_ResumeMusic(); }
    void resumeSound() const { Mix_Resume(-1); }

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
    void renderTexture(const Texture& texture, const glm::vec2& position, const glm::vec2& size,
                       const glm::vec2 mask = {1.0, 1.0}) const;
    void renderFillCircle(const std::string& file_path, const glm::vec2& position, const glm::vec2& size,
                          float alpha = 1.F) const;
    void renderHBar(const glm::vec2& position, const glm::vec2& size, float percent, SDL_FColor color) const;

    void drawGrid(const glm::vec2& top_left, const glm::vec2& bottom_right, float grid_width, float grid_height,
                  SDL_FColor color) const;
    void drawBoundary(const glm::vec2& top_left, const glm::vec2& bottom_right, float boundary_width,
                      SDL_FColor color) const;

    // text
    TTF_Text* createTTF_Text(std::string_view text, const std::string& font_path, int font_size) const;

    // util
    bool isInRect(const glm::vec2& position, const glm::vec2& top_left, const glm::vec2& bottom_right) const;
    bool isMouseInRect(const glm::vec2& top_left, const glm::vec2& bottom_right) const;
    std::string loadTextFile(std::string_view file_path);

private:
    void clean();

    void handleEvents();
    void update(std::chrono::duration<float> delta);
    void render();

public:
    static constexpr double kFps = 60;

private:
    SDL_Window* window_{};
    SDL_Renderer* renderer_{};
    TTF_TextEngine* ttf_engine_{};

    bool is_running_{};

    std::string title_;
    glm::vec2 screen_size_;

    glm::vec2 mouse_position_;
    SDL_MouseButtonFlags mouse_button_flags_;

    std::unique_ptr<Scene> current_scene_;
    std::unique_ptr<Scene> next_scene_;
    std::unique_ptr<AssetStore> asset_store_;

    // 随机数生成器
    std::mt19937 gen_ = std::mt19937(std::random_device{}());

    int score_{};
    int high_score_{};
};

}  // namespace sdl3
}  // namespace pyc
