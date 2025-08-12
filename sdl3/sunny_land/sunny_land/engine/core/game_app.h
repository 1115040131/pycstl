#pragma once

#include <chrono>
#include <memory>

#include "common/noncopyable.h"

struct SDL_Window;
struct SDL_Renderer;

namespace pyc::sunny_land {

class Time;
class ResourceManager;

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
    void update(std::chrono::duration<double> delta_time);
    void render();
    void close();

#pragma region init
    // 各模块的初始化/创建函数，在init()中调用
    [[nodiscard]] bool initSDL();
    [[nodiscard]] bool initTime();
    [[nodiscard]] bool initResourceManager();
#pragma endregion

    // 测试函数
    void testResourceManger();

private:
    SDL_Window* window_{};
    SDL_Renderer* renderer_{};
    bool is_running_{};

    // 引擎组件
    std::unique_ptr<Time> time_;
    std::unique_ptr<ResourceManager> resource_manager_;
};

}  // namespace pyc::sunny_land