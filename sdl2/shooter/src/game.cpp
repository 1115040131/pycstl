#include "shooter/game.h"

#include <chrono>
#include <thread>

#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <fmt/base.h>

#include "shooter/scene_main.h"
#include "shooter/scene_title.h"

namespace pyc {
namespace sdl2 {

using namespace std::chrono_literals;

void Game::run() {
    constexpr auto kFrameTime = 1s / kFps;

    auto last = std::chrono::steady_clock::now();

    while (is_running_) {
        auto start = std::chrono::steady_clock::now();

        SDL_Event event{};
        handleEvent(&event);

        update(start - last);
        last = start;

        render();

        auto end = std::chrono::steady_clock::now();
        auto elapsed = end - start;
        if (elapsed < kFrameTime) {
            std::this_thread::sleep_for(kFrameTime - elapsed);
        }
    }
}

void Game::init() {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        fmt::println("SDL_Init: {}", SDL_GetError());
        return;
    }

    window_ = SDL_CreateWindow("Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kWindowWidth,
                               kWindowHeight, SDL_WINDOW_SHOWN);
    if (!window_) {
        fmt::println("SDL_CreateWindow: {}", SDL_GetError());
        return;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        fmt::println("SDL_CreateRenderer: {}", SDL_GetError());
        return;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        fmt::println("IMG_Init: {}", IMG_GetError());
        return;
    }

    // 初始化 SDL_Mixer
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) != (MIX_INIT_MP3 | MIX_INIT_OGG)) {
        fmt::println("Mix_Init: {}", Mix_GetError());
        return;
    }
    // 打开音频设备
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fmt::println("Mix_OpenAudio: {}", Mix_GetError());
        return;
    }
    // 设置 channel 数量
    Mix_AllocateChannels(32);
    // 设置音量
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
    Mix_Volume(-1, MIX_MAX_VOLUME / 4);

    if (TTF_Init() == -1) {
        fmt::println("TTF_Init: {}", TTF_GetError());
        return;
    }

    // 初始化背景
    near_stars_.texture = IMG_LoadTexture(renderer_, ASSET("image/Stars-A.png"));
    if (!near_stars_.texture) {
        fmt::println("IMG_LoadTexture: {}", IMG_GetError());
        return;
    }
    SDL_QueryTexture(near_stars_.texture, nullptr, nullptr, &near_stars_.width, &near_stars_.height);
    near_stars_.width /= 2;
    near_stars_.height /= 2;

    far_stars_.texture = IMG_LoadTexture(renderer_, ASSET("image/Stars-B.png"));
    if (!far_stars_.texture) {
        fmt::println("IMG_LoadTexture: {}", IMG_GetError());
        return;
    }
    SDL_QueryTexture(far_stars_.texture, nullptr, nullptr, &far_stars_.width, &far_stars_.height);
    far_stars_.width /= 2;
    far_stars_.height /= 2;
    far_stars_.speed = 20;

    // 载入字体
    title_font_ = TTF_OpenFont(ASSET("font/VonwaonBitmap-16px.ttf"), 64);
    if (!title_font_) {
        fmt::println("TTF_OpenFont: {}", TTF_GetError());
        return;
    }
    text_font_ = TTF_OpenFont(ASSET("font/VonwaonBitmap-16px.ttf"), 32);
    if (!text_font_) {
        fmt::println("TTF_OpenFont: {}", TTF_GetError());
        return;
    }

    is_running_ = true;
    // changeScene(std::make_unique<SceneMain>());
    changeScene(std::make_unique<SceneTitle>());
}

void Game::clean() {
    is_running_ = false;
    if (current_scene_) {
        current_scene_->clean();
    }

    SDL_DestroyTexture(near_stars_.texture);
    SDL_DestroyTexture(far_stars_.texture);

    TTF_CloseFont(title_font_);
    TTF_CloseFont(text_font_);

    IMG_Quit();

    Mix_CloseAudio();
    Mix_Quit();

    TTF_Quit();

    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Game::insertLeaderBoard(const std::string& name, int score) {
    leader_board_.insert({score, name});
    if (leader_board_.size() > 8) {
        leader_board_.erase(std::prev(leader_board_.end()));
    }
}

void Game::changeScene(std::unique_ptr<Scene> scene) {
    if (current_scene_) {
        current_scene_->clean();
    }
    current_scene_ = std::move(scene);
    current_scene_->init();
}

SDL_Point Game::renderTextCentered(std::string_view text, float y_percentage, TTF_Font* font) {
    auto surface = TTF_RenderUTF8_Blended(font, text.data(), {255, 255, 255, 255});
    auto texture = SDL_CreateTextureFromSurface(renderer_, surface);
    int y = static_cast<int>((Game::kWindowHeight - surface->h) * y_percentage) - surface->h / 2;
    SDL_Rect rect{Game::kWindowWidth / 2 - surface->w / 2, y, surface->w, surface->h};
    SDL_RenderCopy(renderer_, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    return {rect.x + rect.w, y};
}

void Game::renderText(std::string_view text, SDL_Point position, TTF_Font* font, bool is_left) {
    auto surface = TTF_RenderUTF8_Blended(font, text.data(), {255, 255, 255, 255});
    auto texture = SDL_CreateTextureFromSurface(renderer_, surface);
    auto rect = is_left ? SDL_Rect{position.x, position.y, surface->w, surface->h}
                        : SDL_Rect{kWindowWidth - position.x - surface->w, position.y, surface->w, surface->h};
    SDL_RenderCopy(renderer_, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Game::update(std::chrono::duration<double> delta) {
    backgroundUpdate(delta);
    current_scene_->update(delta);
}

void Game::render() {
    SDL_RenderClear(renderer_);
    backgroundRender();
    current_scene_->render();
    SDL_RenderPresent(renderer_);
}

void Game::handleEvent(SDL_Event* event) {
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT) {
            is_running_ = false;
        }
        current_scene_->handleEvent(event);
    }
}

void Game::backgroundUpdate(std::chrono::duration<double> delta) {
    near_stars_.offset += near_stars_.speed * delta.count();
    if (near_stars_.offset > 0) {
        near_stars_.offset -= near_stars_.height;
    }

    far_stars_.offset += far_stars_.speed * delta.count();
    if (far_stars_.offset > 0) {
        far_stars_.offset -= far_stars_.height;
    }
}

void Game::backgroundRender() {
    for (int y = far_stars_.offset; y < kWindowHeight; y += far_stars_.height) {
        for (int x = 0; x < kWindowWidth; x += far_stars_.width) {
            SDL_Rect dst{x, y, far_stars_.width, far_stars_.height};
            SDL_RenderCopy(renderer_, far_stars_.texture, nullptr, &dst);
        }
    }
    for (int y = near_stars_.offset; y < kWindowHeight; y += near_stars_.height) {
        for (int x = 0; x < kWindowWidth; x += near_stars_.width) {
            SDL_Rect dst{x, y, near_stars_.width, near_stars_.height};
            SDL_RenderCopy(renderer_, near_stars_.texture, nullptr, &dst);
        }
    }
}

}  // namespace sdl2
}  // namespace pyc