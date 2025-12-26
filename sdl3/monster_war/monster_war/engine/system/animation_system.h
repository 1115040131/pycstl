#pragma once

#include <chrono>

#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

/**
 * @brief 动画系统
 *
 * 负责更新实体的动画组件，并同步到精灵组件。
 */
class AnimationSystem {
public:
    AnimationSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~AnimationSystem();

    /**
     * @brief 更新所有拥有动画和精灵组件的实体
     * @param registry entt注册表
     * @param delta_time 增量时间
     */
    void update(std::chrono::duration<float> delta_time);

private:
    void onPlayAnimationEvent(const PlayAnimationEvent& event);  ///< @brief 播放动画事件处理函数

private:
    // 将依赖保存为成员变量，方便回调函数使用
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
};

}  // namespace pyc::monster_war