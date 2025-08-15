#include "sunny_land/engine/core/game_app.h"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/time.h"
#include "sunny_land/engine/render/camera.h"
#include "sunny_land/engine/render/renderer.h"
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
    if (!initSDL() || !initTime() || !initResourceManager() || !initRenderer() || !initCamera()) {
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
    testCamera();  // TODO: remove
}

void GameApp::render() {
    renderer_->clearScreen();

    testRenderer();  // TODO: remove

    renderer_->present();
}

void GameApp::close() {
    spdlog::trace("关闭 GamApp ...");

    // 为了确保正确的销毁顺序，有些智能指针对象也需要手动管理
    resource_manager_.reset();

    if (sdl_renderer_) {
        SDL_DestroyRenderer(sdl_renderer_);
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

    SDL_CreateWindowAndRenderer("SunnyLand", 1280, 720, SDL_WINDOW_RESIZABLE, &window_, &sdl_renderer_);
    if (!window_ || !sdl_renderer_) {
        spdlog::error("无法创建窗口与渲染器! SDL错误: {}", SDL_GetError());
        return false;
    }

    // 设置逻辑分辨率
    SDL_SetRenderLogicalPresentation(sdl_renderer_, 640, 360, SDL_LOGICAL_PRESENTATION_LETTERBOX);
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
        resource_manager_ = std::make_unique<ResourceManager>(sdl_renderer_);
    } catch (const std::exception& e) {
        spdlog::error("初始化资源管理器失败: {}", e.what());
        return false;
    }
    spdlog::trace("资源管理器初始化成功。");
    return true;
}

bool GameApp::initRenderer() {
    try {
        renderer_ = std::make_unique<Renderer>(sdl_renderer_, resource_manager_.get());
    } catch (const std::exception& e) {
        spdlog::error("初始化渲染器失败: {}", e.what());
        return false;
    }
    spdlog::trace("渲染器初始化成功。");
    return true;
}

bool GameApp::initCamera() {
    try {
        camera_ = std::make_unique<Camera>(glm::vec2(640, 360));
    } catch (const std::exception& e) {
        spdlog::error("初始化相机失败: {}", e.what());
        return false;
    }
    spdlog::trace("相机初始化成功。");
    return true;
}
#pragma endregion

#pragma region test
void GameApp::testResourceManger() {
    resource_manager_->getTexture(ASSET("textures/Actors/eagle-attack.png"));
    resource_manager_->getFont(ASSET("fonts/VonwaonBitmap-16px.ttf"), 16);
    resource_manager_->getSound(ASSET("audio/button_click.wav"));

    resource_manager_->unloadTexture(ASSET("textures/Actors/eagle-attack.png"));
    resource_manager_->unloadFont(ASSET("fonts/VonwaonBitmap-16px.ttf"), 16);
    resource_manager_->unloadSound(ASSET("audio/button_click.wav"));
}

void GameApp::testRenderer() {
    Sprite sprite_world(ASSET("textures/Actors/frog.png"));
    Sprite sprite_ui(ASSET("textures/UI/buttons/Start1.png"));
    Sprite sprite_parallax(ASSET("textures/Layers/back.png"));

    static float rotation{};
    rotation += 0.1f;

    renderer_->drawParallax(*camera_, sprite_parallax, glm::vec2(100, 100), glm::vec2(0.5f, 0.5f),
                            glm::bvec2(true, false));
    renderer_->drawSprite(*camera_, sprite_world, glm::vec2(200, 200), glm::vec2(1.0f, 1.0f), rotation);
    renderer_->drawUISprite(sprite_ui, glm::vec2(100, 100));
}

void GameApp::testCamera() {
    auto key_state = SDL_GetKeyboardState(nullptr);
    if (key_state[SDL_SCANCODE_UP]) camera_->move(glm::vec2(0, -1));
    if (key_state[SDL_SCANCODE_DOWN]) camera_->move(glm::vec2(0, 1));
    if (key_state[SDL_SCANCODE_LEFT]) camera_->move(glm::vec2(-1, 0));
    if (key_state[SDL_SCANCODE_RIGHT]) camera_->move(glm::vec2(1, 0));
}
#pragma endregion

}  // namespace pyc::sunny_land