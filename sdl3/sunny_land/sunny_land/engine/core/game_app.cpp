#include "sunny_land/engine/core/game_app.h"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/time.h"
#include "sunny_land/engine/resource/resource_manager.h"

namespace pyc::sunny_land {

using namespace std::chrono_literals;

GameApp::GameApp() = default;

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
    if (!initSDL()) {
        return false;
    }
    if (!initTime()) {
        return false;
    }
    if (!initResourceManager()) {
        return false;
    }

    testResourceManger();  // TODO: remove

    is_running_ = true;
    spdlog::trace("GameApp 初始化成功。");
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

    // 为了确保正确的销毁顺序，有些智能指针对象也需要手动管理
    resource_manager_.reset();

    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
    is_running_ = false;
}

#pragma region init
bool GameApp::initSDL() {
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        spdlog::error("SDL 初始化失败! SDL错误: {}", SDL_GetError());
        return false;
    }

    SDL_CreateWindowAndRenderer("SunnyLand", 1280, 720, SDL_WINDOW_RESIZABLE, &window_, &renderer_);
    if (!window_ || !renderer_) {
        spdlog::error("无法创建窗口与渲染器! SDL错误: {}", SDL_GetError());
        return false;
    }
    spdlog::trace("SDL 初始化成功。");
    return true;
}

bool GameApp::initTime() {
    try {
        time_ = std::make_unique<Time>();
    } catch (const std::exception& e) {
        spdlog::error("初始化时间管理失败: {}", e.what());
        return false;
    }
    spdlog::trace("时间管理初始化成功。");
    return true;
}
bool GameApp::initResourceManager() {
    try {
        resource_manager_ = std::make_unique<ResourceManager>(renderer_);
    } catch (const std::exception& e) {
        spdlog::error("初始化资源管理器失败: {}", e.what());
        return false;
    }
    spdlog::trace("资源管理器初始化成功。");
    return true;
}
#pragma endregion

void GameApp::testResourceManger() {
    resource_manager_->getTexture(ASSET("textures/Actors/eagle-attack.png"));
    resource_manager_->getFont(ASSET("fonts/VonwaonBitmap-16px.ttf"), 16);
    resource_manager_->getSound(ASSET("audio/button_click.wav"));

    resource_manager_->unloadTexture(ASSET("textures/Actors/eagle-attack.png"));
    resource_manager_->unloadFont(ASSET("fonts/VonwaonBitmap-16px.ttf"), 16);
    resource_manager_->unloadSound(ASSET("audio/button_click.wav"));
}

}  // namespace pyc::sunny_land