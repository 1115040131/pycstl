#pragma once

#include <memory>

#include "common/noncopyable.h"
#include "sunny_land/engine/core/time.h"

class SDL_Window;
class SDL_Renderer;

namespace pyc::sunny_land {

/**
 * @brief 主游戏应用程序类, 初始化 SDL, 管理游戏循环
 */
class GameApp final : Noncopyable {
public:
    GameApp() = default;
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

private:
    SDL_Window* window_{};
    SDL_Renderer* renderer_{};
    bool is_running_{};

    // 引擎组件
    std::unique_ptr<Time> time_ = std::make_unique<Time>();
};

}  // namespace pyc::sunny_land