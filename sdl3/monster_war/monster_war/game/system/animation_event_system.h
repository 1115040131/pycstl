#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace pyc::monster_war {

class AnimationEvent;

/**
 * @brief 动画事件系统，用于处理各种动画事件
 */
class AnimationEventSystem {
public:
    AnimationEventSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~AnimationEventSystem();

private:
    // 事件回调函数
    void onAnimationEvent(const AnimationEvent& event);

    // 拆分不同的事件类型
    void handleHitEvent(const AnimationEvent& event);   ///< @brief 命中事件
    void handleEmitEvent(const AnimationEvent& event);  ///< @brief 发射事件

private:
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
};

}  // namespace pyc::monster_war
