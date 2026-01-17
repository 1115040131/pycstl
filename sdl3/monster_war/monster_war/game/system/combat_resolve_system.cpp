#include "monster_war/game/system/combat_resolve_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/component/transform_component.h"
#include "monster_war/game/component/blocked_by_component.h"
#include "monster_war/game/component/blocker_component.h"
#include "monster_war/game/component/class_name_component.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/data/game_stats.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

CombatResolveSystem::CombatResolveSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<AttackEvent>().connect<&CombatResolveSystem::onAttackEvent>(this);
    dispatcher_.sink<HealEvent>().connect<&CombatResolveSystem::onHealEvent>(this);
}

CombatResolveSystem::~CombatResolveSystem() { dispatcher_.disconnect(this); }

void CombatResolveSystem::onAttackEvent(const AttackEvent& event) {
    // 如果目标无效或标记死亡，直接返回
    if (!registry_.valid(event.target_) || registry_.all_of<DeadTag>(event.target_)) {
        return;
    }
    auto& target_stats = registry_.get<StatsComponent>(event.target_);
    float damage = calculateEffectiveDamage(event.damage_, target_stats.def_);
    target_stats.hp_ -= damage;

    if (registry_.all_of<PlayerComponent>(event.target_)) {  // 如果目标是玩家
        spdlog::info("玩家 ID: {} 受到 ID: {} 的伤害, 剩余生命值: {}", entt::to_integral(event.target_),
                     entt::to_integral(event.attacker_), target_stats.hp_);
        if (target_stats.hp_ <= 0) {  //  死亡情况
            target_stats.hp_ = 0;
            // 发送移除单位事件
            dispatcher_.enqueue(RemovePlayerUnitEvent{event.target_});
            spdlog::info("玩家 ID: {} 死亡", entt::to_integral(event.target_));
        } else if (target_stats.hp_ < target_stats.max_hp_) {  // 受伤情况
            registry_.emplace_or_replace<InjuredTag>(event.target_);
        }
    } else if (registry_.all_of<EnemyComponent>(event.target_)) {  // 如果目标是敌人
        spdlog::info("敌人 ID: {} 受到 ID: {} 的伤害, 剩余生命值: {}", entt::to_integral(event.target_),
                     entt::to_integral(event.attacker_), target_stats.hp_);
        if (target_stats.hp_ <= 0) {  //  死亡情况
            target_stats.hp_ = 0;
            registry_.emplace_or_replace<DeadTag>(event.target_);
            spdlog::info("敌人 ID: {} 死亡", entt::to_integral(event.target_));

            // 发送死亡特效事件，需要先获取class_id、位置和是否翻转
            const auto [class_name, transform, sprite] =
                registry_.get<ClassNameComponent, TransformComponent, SpriteComponent>(event.target_);
            dispatcher_.enqueue(
                EnemyDeadEffectEvent{class_name.class_id_, transform.position_, sprite.sprite_.is_flipped_});

            // 更新统计信息
            auto& game_stats = registry_.ctx().get<GameStats&>();
            game_stats.enemy_killed_count_++;  // 敌人击杀数量+1
            if ((game_stats.enemy_killed_count_ + game_stats.enemy_arrived_count_) >= game_stats.enemy_count_) {
                spdlog::warn("敌人全部死亡");
                // TODO: 切换场景逻辑
            }

            // 如果敌人被阻挡，减少阻挡者的阻挡计数
            if (auto blocked_by = registry_.try_get<BlockedByComponent>(event.target_)) {
                if (registry_.valid(blocked_by->entity_)) {
                    auto& blocker = registry_.get<BlockerComponent>(blocked_by->entity_);
                    blocker.current_count_ = std::max(0, blocker.current_count_ - 1);
                }
            }
        } else if (target_stats.hp_ < target_stats.max_hp_) {  // 受伤情况
            registry_.emplace_or_replace<InjuredTag>(event.target_);
        }
    }
}

void CombatResolveSystem::onHealEvent(const HealEvent& event) {
    if (!registry_.valid(event.target_)) {
        return;
    }
    if (!registry_.all_of<PlayerComponent>(event.target_)) {
        return;
    }

    auto& target_stats = registry_.get<StatsComponent>(event.target_);
    target_stats.hp_ += event.amount_;
    spdlog::info("治疗者 ID: {}, 治疗目标 ID: {}, 治疗量: {}", entt::to_integral(event.healer_),
                 entt::to_integral(event.target_), event.amount_);
    // 如果治疗后满血，移除受伤标签
    if (target_stats.hp_ >= target_stats.max_hp_) {
        target_stats.hp_ = target_stats.max_hp_;
        registry_.remove<InjuredTag>(event.target_);
    }
}

float CombatResolveSystem::calculateEffectiveDamage(float attacker_atk, float target_def) {
    return std::max(attacker_atk - target_def, attacker_atk * 0.1f);
}

}  // namespace pyc::monster_war