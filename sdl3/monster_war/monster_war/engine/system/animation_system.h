#pragma once

#include <chrono>

#include <entt/entity/fwd.hpp>

namespace pyc::monster_war {

/**
 * @brief 动画系统
 *
 * 负责更新实体的动画组件，并同步到精灵组件。
 */
class AnimationSystem {
public:
    /**
     * @brief 更新所有拥有动画和精灵组件的实体
     * @param registry entt注册表
     * @param delta_time 增量时间
     */
    void update(entt::registry& registry, std::chrono::duration<float> delta_time);
};

}  // namespace pyc::monster_war