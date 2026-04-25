#pragma once

#include <chrono>

#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace pyc::monster_war {

class EntityFactory;
struct EmitProjectileEvent;

/**
 * @brief 投射物系统
 * 1. 相响应投射物创建事件，创建投射物实体
 * 2. 更新投射物的飞行状态，并发送攻击事件和播放音效
 */
class ProjectileSystem {
public:
    ProjectileSystem(entt::registry& registry, entt::dispatcher& dispatcher, EntityFactory& entity_factory);
    ~ProjectileSystem();

    void update(std::chrono::duration<float> delta_time);

private:
    // 事件回调函数
    void onEmitProjectileEvent(const EmitProjectileEvent& event);

private:
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
    EntityFactory& entity_factory_;  ///< @brief 需要传入实体工厂引用，负责创建投射物实体
};

}  // namespace pyc::monster_war