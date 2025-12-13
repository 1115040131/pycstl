#include "monster_war/engine/core/game_app.h"

#include <SDL3/SDL.h>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

// 引擎组件
#include "monster_war/engine/audio/audio_player.h"
#include "monster_war/engine/core/config.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/core/game_state.h"
#include "monster_war/engine/core/time.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/render/camera.h"
#include "monster_war/engine/render/renderer.h"
#include "monster_war/engine/render/text_renderer.h"
#include "monster_war/engine/resource/resource_manager.h"
#include "monster_war/engine/scene/scene_manager.h"
#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

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

        handleEvents();
        update(delta_time);
        render();

        // spdlog::info("delta_time: {:.6f}s", delta_time.count());
    }

    close();
}

void GameApp::registerSceneSetup(std::function<void(Context&)> func) {
    scene_setup_func_ = std::move(func);
    spdlog::trace("已注册场景设置函数。");
}

bool GameApp::init() {
    spdlog::trace("初始化 GamApp ...");
    if (!scene_setup_func_) {
        spdlog::error("未注册场景设置函数，无法初始化 GameApp。");
        return false;
    }
    // clang-format off
    if (!initDispatcher() ||
        !initConfig() ||
        !initSDL() ||
        !initTime() ||
        !initResourceManager() ||
        !initAudioPlayer() ||
        !initRenderer() ||
        !initCamera() ||
        !initTextRenderer() ||
        !initInputManager() ||
        !initGameState() ||
        !initContext() ||
        !initSceneManager()) {
        return false;
    }
    // clang-format on

    // 调用场景设置函数 (创建第一个场景并压入栈)
    scene_setup_func_(*context_);

    // 注册退出事件 (回调函数可以无参, 代表不使用事件结构体中的数据)
    dispatcher_->sink<QuitEvent>().connect<&GameApp::onQuitEvent>(this);

    is_running_ = true;
    spdlog::trace("GameApp 初始化成功。");
    return true;
}

void GameApp::handleEvents() {
    // 处理并分发事件
    input_manager_->update();

    scene_manager_->handleInput();
}

void GameApp::update(std::chrono::duration<float> delta_time) {
    // 游戏逻辑更新
    scene_manager_->update(delta_time);

    // 分发事件
    dispatcher_->update();
}

void GameApp::render() {
    renderer_->clearScreen();

    scene_manager_->render();

    renderer_->present();
}

void GameApp::close() {
    spdlog::trace("关闭 GamApp ...");

    // 断开事件处理函数
    dispatcher_->sink<QuitEvent>().disconnect<&GameApp::onQuitEvent>(this);

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
bool GameApp::initDispatcher() {
    try {
        dispatcher_ = std::make_unique<entt::dispatcher>();
    } catch (const std::exception& e) {
        spdlog::error("初始化事件分发器失败: {}", e.what());
        return false;
    }
    spdlog::trace("事件分发器初始化成功。");
    return true;
}

bool GameApp::initConfig() {
    try {
        config_ = std::make_unique<Config>("assets/config.json");
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

    // 设置渲染器支持透明色
    SDL_SetRenderDrawBlendMode(sdl_renderer_, SDL_BLENDMODE_BLEND);

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
    resource_manager_->loadResources("assets/data/resource_mapping.json");  // 载入默认资源映射文件
    return true;
}

bool GameApp::initAudioPlayer() {
    try {
        audio_player_ = std::make_unique<AudioPlayer>(resource_manager_.get());
        // 设置音量
        audio_player_->setMusicVolume(config_->CONFIG(audio.music_volume));
        audio_player_->setSoundVolume(config_->CONFIG(audio.sound_volume));
    } catch (const std::exception& e) {
        spdlog::error("初始化音频播放器失败: {}", e.what());
        return false;
    }
    spdlog::trace("音频播放器初始化成功。");
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

bool GameApp::initTextRenderer() {
    try {
        text_renderer_ = std::make_unique<TextRenderer>(sdl_renderer_, resource_manager_.get());
    } catch (const std::exception& e) {
        spdlog::error("初始化文字渲染引擎失败: {}", e.what());
        return false;
    }
    spdlog::trace("文字渲染引擎初始化成功。");
    return true;
}

bool GameApp::initInputManager() {
    try {
        input_manager_ = std::make_unique<InputManager>(sdl_renderer_, config_.get(), dispatcher_.get());
    } catch (const std::exception& e) {
        spdlog::error("初始化输入管理器失败: {}", e.what());
        return false;
    }
    spdlog::trace("输入管理器初始化成功。");
    return true;
}

bool GameApp::initGameState() {
    try {
        game_state_ = std::make_unique<GameState>(window_, sdl_renderer_);
    } catch (const std::exception& e) {
        spdlog::error("初始化游戏状态失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameApp::initContext() {
    try {
        // clang-format off
        context_ = std::make_unique<Context>(*dispatcher_,
                                             *resource_manager_,
                                             *renderer_,
                                             *camera_,
                                             *text_renderer_,
                                             *input_manager_,
                                             *audio_player_,
                                             *game_state_);
        // clang-format on
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

void GameApp::onQuitEvent() {
    spdlog::info("GameApp 收到来自事件分发器的退出请求。");
    is_running_ = false;
}

}  // namespace pyc::monster_war