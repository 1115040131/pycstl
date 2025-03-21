#include "sdl3/common/core/game.h"

#include <chrono>
#include <thread>

#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <fmt/base.h>

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

using namespace std::chrono_literals;

void Game::init(std::string_view title, int width, int height) {
    title_ = title;
    screen_size_ = glm::vec2(width, height);

    // SDL3 初始化
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        fmt::println("SDL_Init: {}", SDL_GetError());
        return;
    }

    // SDL3_Mixer 初始化
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) != (MIX_INIT_MP3 | MIX_INIT_OGG)) {
        fmt::println("Mix_Init: {}", SDL_GetError());
        return;
    }
    if (!Mix_OpenAudio(0, nullptr)) {
        fmt::println("Mix_OpenAudio: {}", SDL_GetError());
        return;
    }
    Mix_AllocateChannels(16);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
    Mix_Volume(-1, MIX_MAX_VOLUME / 4);

    // SDL3_TTF 初始化
    if (!TTF_Init()) {
        fmt::println("TTF_Init: {}", SDL_GetError());
        return;
    }

    // 创建窗口与渲染器
    SDL_CreateWindowAndRenderer(title.data(), width, height, SDL_WINDOW_RESIZABLE, &window_, &renderer_);
    if (!window_ || !renderer_) {
        fmt::println("SDL_CreateWindowAndRenderer: {}", SDL_GetError());
        return;
    }

    // 设置窗口分辨率
    SDL_SetRenderLogicalPresentation(renderer_, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // 创建 AssetStore
    asset_store_ = std::make_unique<AssetStore>(renderer_);

    is_running_ = true;
}

void Game::clean() {
    if (current_scene_) {
        current_scene_->clean();
    }

    asset_store_.reset();

    // 释放渲染器和窗口
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    // 退出 Mix
    Mix_CloseAudio();
    Mix_Quit();
    // 退出 TTF
    TTF_Quit();
    // 退出 SDL
    SDL_Quit();
}

void Game::run() {
    constexpr auto kFrameTime = 1s / kFps;
    auto last = std::chrono::steady_clock::now();  // 上一帧update时间
    while (is_running_) {
        auto start = std::chrono::steady_clock::now();  // 当前帧时间

        handleEvents();
        update(start - last);
        last = start;
        render();

        auto end = std::chrono::steady_clock::now();
        auto elapsed = end - last;
        if (elapsed < kFrameTime) {
            std::this_thread::sleep_for(kFrameTime - elapsed - 0.1ms);
        }

        {
            auto tmp = std::chrono::steady_clock::now();
            fmt::println("elapsed: {},  FPS: {}", elapsed.count(), 1.0s / (tmp - start));
        }
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                is_running_ = false;
                break;
            default:
                current_scene_->handleEvents(event);
                break;
        }
    }
}

void Game::update(std::chrono::duration<float> delta) { current_scene_->update(delta); }

void Game::render() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    current_scene_->render();
    SDL_RenderPresent(renderer_);
}

void Game::changeScene(std::unique_ptr<Scene> scene) {
    if (current_scene_) {
        current_scene_->clean();
    }
    current_scene_ = std::move(scene);
    if (current_scene_) {
        current_scene_->init();
    }
}

void Game::drawGrid(const glm::vec2& top_left, const glm::vec2& bottom_right, float grid_width, float grid_height,
                    SDL_FColor color) {
    SDL_SetRenderDrawColorFloat(renderer_, color.r, color.g, color.b, color.a);
    for (float x = top_left.x; x <= bottom_right.x; x += grid_width) {
        SDL_RenderLine(renderer_, x, top_left.y, x, bottom_right.y);
    }
    for (float y = top_left.y; y <= bottom_right.y; y += grid_height) {
        SDL_RenderLine(renderer_, top_left.x, y, bottom_right.x, y);
    }
    SDL_SetRenderDrawColorFloat(renderer_, 0, 0, 0, 1);
}

void Game::drawBoundary(const glm::vec2& top_left, const glm::vec2& bottom_right, float boundary_width,
                        SDL_FColor color) {
    SDL_SetRenderDrawColorFloat(renderer_, color.r, color.g, color.b, color.a);
    for (float i = 0; i < boundary_width; i++) {
        SDL_FRect rect = {
            top_left.x - i,
            top_left.y - i,
            bottom_right.x - top_left.x + 2 * i,
            bottom_right.y - top_left.y + 2 * i,
        };
        SDL_RenderRect(renderer_, &rect);
    }
    SDL_SetRenderDrawColorFloat(renderer_, 0, 0, 0, 1);
}

}  // namespace sdl3
}  // namespace pyc