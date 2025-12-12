#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include <entt/signal/fwd.hpp>

#include "common/noncopyable.h"

struct SDL_Window;
struct SDL_Renderer;

namespace pyc::monster_war {

class Config;
class Time;
class ResourceManager;
class AudioPlayer;
class Renderer;
class Camera;
class TextRenderer;
class InputManager;
class GameState;
class Context;
class SceneManager;

/**
 * @brief 主游戏应用程序类, 初始化 SDL, 管理游戏循环
 */
class GameApp final : Noncopyable {
public:
    GameApp();
    ~GameApp();

    /**
     * @brief 运行游戏应用程序
     */
    void run();

    /**
     * @brief 注册用于设置初始游戏场景的函数。
     *        这个函数将在 SceneManager 初始化后被调用。
     * @param func 一个接收 Context 引用的函数对象。
     */
    void registerSceneSetup(std::function<void(Context&)> func);

private:
    [[nodiscard]] bool init();
    void handleEvents();
    void update(std::chrono::duration<float> delta_time);
    void render();
    void close();

#pragma region init
    // 各模块的初始化/创建函数，在init()中调用
    [[nodiscard]] bool initDispatcher();
    [[nodiscard]] bool initConfig();
    [[nodiscard]] bool initSDL();
    [[nodiscard]] bool initTime();
    [[nodiscard]] bool initResourceManager();
    [[nodiscard]] bool initAudioPlayer();
    [[nodiscard]] bool initRenderer();
    [[nodiscard]] bool initCamera();
    [[nodiscard]] bool initTextRenderer();
    [[nodiscard]] bool initInputManager();
    [[nodiscard]] bool initGameState();

    [[nodiscard]] bool initContext();
    [[nodiscard]] bool initSceneManager();
#pragma endregion

    // 事件处理函数
    void onQuitEvent();

private:
    SDL_Window* window_{};
    SDL_Renderer* sdl_renderer_{};
    bool is_running_{};

    /// @brief 游戏场景设置函数，用于在运行游戏前设置初始场景 (GameApp不再决定初始场景是什么)
    std::function<void(Context&)> scene_setup_func_;

    // 引擎组件
    std::unique_ptr<entt::dispatcher> dispatcher_;
    std::unique_ptr<Config> config_;
    std::unique_ptr<Time> time_;
    std::unique_ptr<ResourceManager> resource_manager_;
    std::unique_ptr<AudioPlayer> audio_player_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<TextRenderer> text_renderer_;
    std::unique_ptr<InputManager> input_manager_;
    std::unique_ptr<GameState> game_state_;

    std::unique_ptr<Context> context_;
    std::unique_ptr<SceneManager> scene_manager_;
};

}  // namespace pyc::monster_war