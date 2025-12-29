#pragma once

#include <chrono>

#include <entt/entity/fwd.hpp>

namespace pyc::monster_war {

/**
 * @brief 计时器系统，用于更新所有包含计时器的组件，
 * 并在满足条件时添加必要的标签，（如攻击冷却完成后，添加“可攻击”标签）。
 */
class TimerSystem {
public:
    void update(entt::registry& registry, std::chrono::duration<float> delta_time);

private:
    // 拆分逻辑的函数，在update中调用
    void updateAttackTimer(entt::registry& registry,
                           std::chrono::duration<float> delta_time);  ///< @brief 处理攻击计时器
    // TODO: 处理其他计时器
};

}  // namespace pyc::monster_war