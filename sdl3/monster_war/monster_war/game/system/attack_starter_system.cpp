#include "monster_war/game/system/attack_starter_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include "monster_war/engine/component/velocity_component.h"
#include "monster_war/engine/utils/events.h"
#include "monster_war/game/component/blocked_by_component.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/component/target_component.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

using namespace entt::literals;

void AttackStarterSystem::update(entt::registry& registry, entt::dispatcher& dispatcher) {
    updateEnemyBlocked(registry, dispatcher);
    updateEnemyRanged(registry, dispatcher);
    updatePlayer(registry, dispatcher);
}

void AttackStarterSystem::updateEnemyBlocked(entt::registry& registry, entt::dispatcher& dispatcher) {
    // 筛选条件：被阻挡的敌人，攻击冷却完毕（有“可攻击”标签）
    auto view_blocked_enemies = registry.view<BlockedByComponent, EnemyComponent, AttackReadyTag>();
    for (auto [entity, blocked_by, enemy] : view_blocked_enemies.each()) {
        // 添加“动作锁定”标签，防止敌人继续移动（确保攻击动画执行完毕再进行其他动作）
        registry.emplace_or_replace<ActionLockTag>(entity);
        // 每次攻击后，移除“可攻击”标签，攻击冷却重新计时
        registry.remove<AttackReadyTag>(entity);
        dispatcher.enqueue(PlayAnimationEvent{entity, "attack"_hs, false});
    }
}

void AttackStarterSystem::updateEnemyRanged(entt::registry& registry, entt::dispatcher& dispatcher) {
    // 筛选条件：有目标的远程敌人，未被阻挡，攻击冷却完毕（有“可攻击”标签）
    auto view_enemy_ranged =
        registry.view<EnemyComponent, TargetComponent, AttackReadyTag>(entt::exclude<BlockedByComponent>);
    for (auto [entity, enemy, target] : view_enemy_ranged.each()) {
        registry.emplace_or_replace<ActionLockTag>(entity);
        // 对于体积很小的组件，可以直接构造替换，不必“获取 + 修改”
        registry.emplace_or_replace<VelocityComponent>(entity, glm::vec2(0.0f, 0.0f));
        registry.remove<AttackReadyTag>(entity);
        dispatcher.enqueue(PlayAnimationEvent{entity, "ranged_attack"_hs, false});
    }
}

void AttackStarterSystem::updatePlayer(entt::registry& registry, entt::dispatcher& dispatcher) {
    // 筛选条件：有目标的玩家，攻击冷却完毕（有“可攻击”标签）
    auto view_player = registry.view<PlayerComponent, TargetComponent, AttackReadyTag>();
    for (auto [entity, player, target] : view_player.each()) {
        /* 玩家静止不动，不需要添加动作锁定标签 */
        registry.remove<AttackReadyTag>(entity);
        if (registry.all_of<HealerTag>(entity)) {
            dispatcher.enqueue(PlayAnimationEvent{entity, "heal"_hs, false});
        } else {
            dispatcher.enqueue(PlayAnimationEvent{entity, "attack"_hs, false});
        }
    }
}

}  // namespace pyc::monster_war