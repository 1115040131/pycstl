module;

#include <chrono>
#include <functional>
#include <memory>

struct SDL_Window;
struct SDL_Renderer;

export module sunny_land.engine.core.game_app;

import common.noncopyable;
import sunny_land.engine.audio.audio_player;
import sunny_land.engine.core;
import sunny_land.engine.core.config;
import sunny_land.engine.core.game_state;
import sunny_land.engine.core.time;
import sunny_land.engine.input.input_manager;
import sunny_land.engine.resource;
import sunny_land.engine.scene;

export namespace pyc::sunny_land {

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
     * @param func 一个接收 SceneManager 引用的函数对象。
     */
    void registerSceneSetup(std::function<void(SceneManager&)> func);

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

    /// @brief 游戏场景设置函数，用于在运行游戏前设置初始场景 (GameApp不再决定初始场景是什么)
    std::function<void(SceneManager&)> scene_setup_func_;

    // 引擎组件
    Config config_;
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
