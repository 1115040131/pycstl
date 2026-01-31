#include "monster_war/game/system/block_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/component/velocity_component.h"
#include "monster_war/engine/utils/events.h"
#include "monster_war/engine/utils/math.h"
#include "monster_war/game/component/blocked_by_component.h"
#include "monster_war/game/component/blocker_component.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/def/constants.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

using namespace entt::literals;

void BlockSystem::update(entt::registry& registry, entt::dispatcher& dispatcher) {
    spdlog::trace("BlockSystem::update");

    // --- 检查阻挡者是否依然有效 ---
    for (auto [entity, blocked_by] : registry.view<BlockedByComponent>().each()) {
        // 如果BlockedBy指向的实体无效(例如死亡)，移除被阻挡组件，并发送播放动画“walk”事件
        if (!registry.valid(blocked_by.entity_)) {
            spdlog::info("阻挡者: ID: {}, 无效, 移除阻挡者组件", entt::to_integral(blocked_by.entity_));
            registry.remove<BlockedByComponent>(entity);
            registry.remove<ActionLockTag>(entity);  // 移除可能存在的动作锁定标签
            dispatcher.enqueue(PlayAnimationEvent{entity, "walk"_hs});
        }
    }

    // --- 判断是否需要添加阻挡者组件 ---
    // 获取所有阻挡者
    auto view_blocker = registry.view<BlockerComponent, TransformComponent>();
    // 遍历所有敌人
    for (auto [enemy_entity, enemy, enemy_transform, enemy_velocity] :
         registry.view<EnemyComponent, TransformComponent, VelocityComponent>(entt::exclude<BlockedByComponent>)
             .each()) {
        for (auto [blocker_entity, blocker, blocker_transform] : view_blocker.each()) {
            // 检查阻挡者是否还能阻挡
            if (blocker.current_count_ >= blocker.max_count_) {
                continue;  // 如果不能阻挡，则跳过
            }

            // 如果被阻挡（检查敌人和阻挡者之间的距离是否小于阻挡半径）
            if (distanceSquared(enemy_transform.position_, blocker_transform.position_) <
                BLOCK_RADIUS * BLOCK_RADIUS) {
                blocker.current_count_++;
                enemy_velocity.velocity_ = glm::vec2(0.0f);  // 停止敌人移动

                // 给敌人添加被阻挡组件
                registry.emplace<BlockedByComponent>(enemy_entity, blocker_entity);
                spdlog::info("敌人: ID: {}, 被阻挡, 阻挡者: ID: {}", entt::to_integral(enemy_entity),
                             entt::to_integral(blocker_entity));

                // 播放动画“attack” （临时测试用，未来会将攻击逻辑放在其他系统里）
                dispatcher.enqueue(PlayAnimationEvent{enemy_entity, "attack"_hs, true});
            }
        }
    }
}

}  // namespace pyc::monster_war