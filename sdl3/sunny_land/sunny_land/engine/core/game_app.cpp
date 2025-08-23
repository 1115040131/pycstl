#include "sunny_land/engine/core/game_app.h"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "sunny_land/engine/utils/macro.h"
#include "sunny_land/game/scene/game_scene.h"

// 引擎组件
#include "sunny_land/engine/core/config.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/core/time.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/physics/physics_engine.h"
#include "sunny_land/engine/render/camera.h"
#include "sunny_land/engine/render/renderer.h"
#include "sunny_land/engine/resource/resource_manager.h"
#include "sunny_land/engine/scene/scene_manager.h"

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

    while (is_running_) {
        time_->update();
        auto delta_time = time_->getDeltaTime();
        input_manager_->update();

        handleEvents();
        update(delta_time);
        render();

        // spdlog::info("delta_time: {:.6f}s", delta_time.count());
    }

    close();
}

bool GameApp::init() {
    spdlog::trace("初始化 GamApp ...");
    if (!initConfig() || !initSDL() || !initTime() || !initResourceManager() || !initRenderer() || !initCamera() ||
        !initInputManager() || !initPhysicsEngine() || !initContext() || !initSceneManager()) {
        return false;
    }

    // 创建第一个场景并压入栈
    auto scene = std::make_unique<GameScene>("GameScene", *context_, *scene_manager_);
    scene_manager_->requestPushScene(std::move(scene));

    is_running_ = true;
    spdlog::trace("GameApp 初始化成功。");
    return true;
}

void GameApp::handleEvents() {
    if (input_manager_->shouldQuit()) {
        spdlog::trace("GameApp 收到来自 InputManager 的退出请求。");
        is_running_ = false;
        return;
    }
    scene_manager_->handleInput();
}

void GameApp::update(std::chrono::duration<float> delta_time) { scene_manager_->update(delta_time); }

void GameApp::render() {
    renderer_->clearScreen();

    scene_manager_->render();

    renderer_->present();
}

void GameApp::close() {
    spdlog::trace("关闭 GamApp ...");
    // 先关闭场景管理器，确保所有场景都被清理
    scene_manager_->clean();

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
bool GameApp::initConfig() {
    try {
        config_ = std::make_unique<Config>(ASSET("config.json"));
    } catch (const std::exception& e) {
        spdlog::error("初始化配置失败: {}", e.what());
        return false;
    }
    spdlog::trace("配置初始化成功。");
    return true;
}

bool GameApp::initSDL() {
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        spdlog::error("SDL 初始化失败! SDL错误: {}", SDL_GetError());
        return false;
    }

    SDL_CreateWindowAndRenderer("SunnyLand", config_->CONFIG(window.width), config_->CONFIG(window.height),
                                SDL_WINDOW_RESIZABLE, &window_, &sdl_renderer_);
    if (!window_ || !sdl_renderer_) {
        spdlog::error("无法创建窗口与渲染器! SDL错误: {}", SDL_GetError());
        return false;
    }

    // 设置 VSync (注意: VSync 开启时，驱动程序会尝试将帧率限制到显示器刷新率，有可能会覆盖我们手动设置的
    // target_fps)
    int vsync_mode = config_->CONFIG(graphics.vsync) ? SDL_RENDERER_VSYNC_ADAPTIVE : SDL_RENDERER_VSYNC_DISABLED;
    SDL_SetRenderVSync(sdl_renderer_, vsync_mode);
    spdlog::trace("VSync 设置为: {}", config_->CONFIG(graphics.vsync) ? "Enabled" : "Disabled");

    // 设置逻辑分辨率
    SDL_SetRenderLogicalPresentation(sdl_renderer_, config_->CONFIG(window.width) / 2,
                                     config_->CONFIG(window.height) / 2, SDL_LOGICAL_PRESENTATION_LETTERBOX);
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
    time_->setTargetFps(config_->CONFIG(performance.target_fps));
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
        camera_ = std::make_unique<Camera>(
            glm::vec2(config_->CONFIG(window.width) / 2, config_->CONFIG(window.height) / 2));
    } catch (const std::exception& e) {
        spdlog::error("初始化相机失败: {}", e.what());
        return false;
    }
    spdlog::trace("相机初始化成功。");
    return true;
}

bool GameApp::initInputManager() {
    try {
        input_manager_ = std::make_unique<InputManager>(sdl_renderer_, config_.get());
    } catch (const std::exception& e) {
        spdlog::error("初始化输入管理器失败: {}", e.what());
        return false;
    }
    spdlog::trace("输入管理器初始化成功。");
    return true;
}

bool GameApp::initPhysicsEngine() {
    try {
        physics_engine_ = std::make_unique<PhysicsEngine>();
    } catch (const std::exception& e) {
        spdlog::error("初始化物理引擎失败: {}", e.what());
        return false;
    }
    spdlog::trace("物理引擎初始化成功。");
    return true;
}

bool GameApp::initContext() {
    try {
        context_ =
            std::make_unique<Context>(*resource_manager_, *renderer_, *camera_, *input_manager_, *physics_engine_);
    } catch (const std::exception& e) {
        spdlog::error("初始化上下文失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameApp::initSceneManager() {
    try {
        scene_manager_ = std::make_unique<SceneManager>(*context_);
    } catch (const std::exception& e) {
        spdlog::error("初始化场景管理器失败: {}", e.what());
        return false;
    }
    spdlog::trace("场景管理器初始化成功。");
    return true;
}
#pragma endregion

}  // namespace pyc::sunny_land