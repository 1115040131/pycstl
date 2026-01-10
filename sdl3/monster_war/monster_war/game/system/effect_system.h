#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace pyc::monster_war {

class EntityFactory;
class EnemyDeadEffectEvent;

/**
 * @brief 特效系统，处理所有特效的创建
 */
class EffectSystem {
public:
    EffectSystem(entt::registry& registry, entt::dispatcher& dispatcher, EntityFactory& entity_factory);
    ~EffectSystem();

private:
    // 事件回调函数
    void onEnemyDeadEffectEvent(const EnemyDeadEffectEvent& event);  ///< @brief 敌人死亡特效事件
    // TODO: 未来添加其他特效事件回调函数

private:
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
    EntityFactory& entity_factory_;
};
}  // namespace pyc::monster_war