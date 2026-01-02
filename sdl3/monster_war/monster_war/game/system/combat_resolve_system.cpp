#include "monster_war/game/system/combat_resolve_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/game/component/blocked_by_component.h"
#include "monster_war/game/component/blocker_component.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

CombatResolveSystem::CombatResolveSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<AttackEvent>().connect<&CombatResolveSystem::onAttackEvent>(this);
    dispatcher_.sink<HealEvent>().connect<&CombatResolveSystem::onHealEvent>(this);
}

CombatResolveSystem::~CombatResolveSystem() { dispatcher_.disconnect(this); }

void CombatResolveSystem::onAttackEvent(const AttackEvent& event) {
    if (!registry_.valid(event.target_)) {
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
            registry_.emplace<DeadTag>(event.target_);
            spdlog::info("玩家 ID: {} 死亡", entt::to_integral(event.target_));
        } else if (target_stats.hp_ < target_stats.max_hp_) {  // 受伤情况
            registry_.emplace_or_replace<InjuredTag>(event.target_);
        }
    } else if (registry_.all_of<EnemyComponent>(event.target_)) {  // 如果目标是敌人
        spdlog::info("敌人 ID: {} 受到 ID: {} 的伤害, 剩余生命值: {}", entt::to_integral(event.target_),
                     entt::to_integral(event.attacker_), target_stats.hp_);
        if (target_stats.hp_ <= 0) {  //  死亡情况
            target_stats.hp_ = 0;
            registry_.emplace<DeadTag>(event.target_);
            spdlog::info("敌人 ID: {} 死亡", entt::to_integral(event.target_));
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