#pragma once

#include <chrono>
#include <memory>

#include "common/noncopyable.h"

struct SDL_Window;
struct SDL_Renderer;

namespace pyc::sunny_land {

class Config;
class Time;
class ResourceManager;
class AudioPlayer;
class Renderer;
class Camera;
class TextRenderer;
class InputManager;
class PhysicsEngine;
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

private:
    [[nodiscard]] bool init();
    void handleEvents();
    void update(std::chrono::duration<float> delta_time);
    void render();
    void close();

#pragma region init
    // 各模块的初始化/创建函数，在init()中调用
    [[nodiscard]] bool initConfig();
    [[nodiscard]] bool initSDL();
    [[nodiscard]] bool initTime();
    [[nodiscard]] bool initResourceManager();
    [[nodiscard]] bool initAudioPlayer();
    [[nodiscard]] bool initRenderer();
    [[nodiscard]] bool initCamera();
    [[nodiscard]] bool initTextRenderer();
    [[nodiscard]] bool initInputManager();
    [[nodiscard]] bool initPhysicsEngine();
    [[nodiscard]] bool initGameState();

    [[nodiscard]] bool initContext();
    [[nodiscard]] bool initSceneManager();
#pragma endregion

private:
    SDL_Window* window_{};
    SDL_Renderer* sdl_renderer_{};
    bool is_running_{};

    // 引擎组件
    std::unique_ptr<Config> config_;
    std::unique_ptr<Time> time_;
    std::unique_ptr<ResourceManager> resource_manager_;
    std::unique_ptr<AudioPlayer> audio_player_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<TextRenderer> text_renderer_;
    std::unique_ptr<InputManager> input_manager_;
    std::unique_ptr<PhysicsEngine> physics_engine_;
    std::unique_ptr<GameState> game_state_;

    std::unique_ptr<Context> context_;
    std::unique_ptr<SceneManager> scene_manager_;
};

}  // namespace pyc::sunny_land