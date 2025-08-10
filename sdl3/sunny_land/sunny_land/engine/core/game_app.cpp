#include "sunny_land/engine/core/game_app.h"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace pyc::sunny_land {

using namespace std::chrono_literals;

GameApp::~GameApp() {
    if (is_running_) {
        spdlog::warn("GameApp 销毁时没有被显示关闭. 现在关闭...");
        close();
    }
}

void GameApp::run() {
    if (!init()) {
        spdlog::error("初始化失败");
        return;
    }

    time_->setTargetFps(144);

    while (is_running_) {
        time_->update();
        auto delta_time = time_->getDeltaTime();

        handleEvents();
        update(delta_time);
        render();

        // spdlog::info("delta_time: {:.6f}s", delta_time.count());
    }

    close();
}

bool GameApp::init() {
    spdlog::trace("初始化 GamApp ...");
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        spdlog::error("SDL 初始化失败! SDL错误: {}", SDL_GetError());
        return false;
    }

    SDL_CreateWindowAndRenderer("SunnyLand", 1280, 720, SDL_WINDOW_RESIZABLE, &window_, &renderer_);
    if (!window_ || !renderer_) {
        spdlog::error("无法创建窗口与渲染器! SDL错误: {}", SDL_GetError());
        return false;
    }

    is_running_ = true;
    return true;
}

void GameApp::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                is_running_ = false;
                break;
        }
    }
}

void GameApp::update(std::chrono::duration<double> /* delta_time */) {
    // TODO
}

void GameApp::render() {
    // TODO
}

void GameApp::close() {
    spdlog::trace("关闭 GamApp ...");
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
    is_running_ = false;
}

}  // namespace pyc::sunny_land