#pragma once

#include <memory>

#include <SDL.h>
#include <SDL_ttf.h>

#include "common/singleton.h"
#include "shooter/object.h"
#include "shooter/scene.h"

#define ASSET_PATH "sdl2/shooter/assets/"
#define ASSET(filename) (ASSET_PATH filename)

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

    TTF_Font* title_font() const { return title_font_; }
    TTF_Font* text_font() const { return text_font_; }

    void changeScene(std::unique_ptr<Scene> scene);
    void renderTextCentered(std::string_view text, float y_percentage, TTF_Font* font);

private:
    void update(std::chrono::duration<double> delta);
    void render();
    void handleEvent(SDL_Event* event);

    void backgroundUpdate(std::chrono::duration<double> delta);

    void backgroundRender();

public:
    static constexpr int kWindowWidth = 600;
    static constexpr int kWindowHeight = 800;
    static constexpr double kFps = 60;

private:
    bool is_running_ = false;
    std::unique_ptr<Scene> current_scene_ = nullptr;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    Background near_stars_;
    Background far_stars_;

    TTF_Font* title_font_ = nullptr;
    TTF_Font* text_font_ = nullptr;
};

}  // namespace sdl2
}  // namespace pyc