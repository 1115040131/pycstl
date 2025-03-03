#include "shooter/game.h"

#include <chrono>
#include <thread>

#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <fmt/base.h>

#include "shooter/scene_main.h"

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

    is_running_ = true;
    changeScene(std::make_unique<SceneMain>());
}

void Game::clean() {
    is_running_ = false;
    if (current_scene_) {
        current_scene_->clean();
    }

    IMG_Quit();

    Mix_CloseAudio();
    Mix_Quit();

    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Game::changeScene(std::unique_ptr<Scene> scene) {
    if (current_scene_) {
        current_scene_->clean();
    }
    current_scene_ = std::move(scene);
    current_scene_->init();
}

void Game::update(std::chrono::duration<double> delta) { current_scene_->update(delta); }

void Game::render() {
    SDL_RenderClear(renderer_);
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

}  // namespace sdl2
}  // namespace pyc