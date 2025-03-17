#pragma once

#include <map>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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

    ~Game();

public:
    void run();

    void init();
    void clean();

    SDL_Window* window() const { return window_; }
    SDL_Renderer* renderer() const { return renderer_; }

    TTF_Font* title_font() const { return title_font_; }
    TTF_Font* text_font() const { return text_font_; }

    int finalScore() const { return final_score_; }
    void setFinalScore(int score) { final_score_ = score; }

    const std::multimap<int, std::string, std::greater<int>>& leaderBoard() const { return leader_board_; }
    void insertLeaderBoard(const std::string& name, int score);

    void changeScene(std::unique_ptr<Scene> scene);
    SDL_Point renderTextCentered(std::string_view text, float y_percentage, TTF_Font* font);
    void renderText(std::string_view text, SDL_Point position, TTF_Font* font, bool is_left = true);

private:
    void update(std::chrono::duration<double> delta);
    void render();
    void handleEvent(SDL_Event* event);

    void backgroundUpdate(std::chrono::duration<double> delta);
    void backgroundRender();

    void saveData();
    void loadData();

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

    int final_score_ = 0;
    std::multimap<int, std::string, std::greater<int>> leader_board_;
};

}  // namespace sdl2
}  // namespace pyc