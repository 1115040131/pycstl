#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace pyc::monster_war {

class EntityFactory;
struct EnemyDeadEffectEvent;
struct EffectEvent;

/**
 * @brief 特效系统，处理所有特效的创建
 */
class EffectSystem {
public:
    EffectSystem(entt::dispatcher& dispatcher, EntityFactory& entity_factory);
    ~EffectSystem();

private:
    // 事件回调函数
    void onEnemyDeadEffectEvent(const EnemyDeadEffectEvent& event);  ///< @brief 敌人死亡特效事件
    void onEffectEvent(const EffectEvent& event);                    ///< @brief (通用)特效事件

private:
    entt::dispatcher& dispatcher_;
    EntityFactory& entity_factory_;
};
}  // namespace pyc::monster_war