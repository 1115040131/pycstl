#pragma once

#include <chrono>
#include <memory>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

class Context;
class PlayerComponent;

/**
 * @brief 玩家状态机的抽象基类。
 */
class PlayerState : Noncopyable {
    friend class PlayerComponent;

public:
    explicit PlayerState(PlayerComponent* player_component) : player_component_(player_component) {}
    virtual ~PlayerState() = default;

    ///< @brief 播放指定名称的动画，使用 AnimationComponent 的方法
    void playAnimation(std::string_view animation_name);

protected:
    // 核心状态方法
    virtual void enter() = 0;                                                                 ///< @brief 进入
    virtual void exit() = 0;                                                                  ///< @brief 离开
    virtual std::unique_ptr<PlayerState> handleInput(Context&) = 0;                           ///< @brief 处理输入
    virtual std::unique_ptr<PlayerState> update(std::chrono::duration<float>, Context&) = 0;  ///< @brief 更新
    /* handleInput 和 update 返回值为下一个状态，如果不需要切换状态，则返回 nullptr */

protected:
    PlayerComponent* player_component_ = nullptr;  ///< @brief 指向拥有此状态的玩家组件
};

}  // namespace pyc::sunny_land