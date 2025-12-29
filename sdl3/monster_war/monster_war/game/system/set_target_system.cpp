#include "monster_war/game/system/set_target_system.h"

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/utils/math.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/component/target_component.h"
#include "monster_war/game/def/constants.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

void SetTargetSystem::update(entt::registry& registry) {
    updateHasTarget(registry);
    updateNoTargetPlayer(registry);
    updateNoTargetEnemy(registry);
    updateHealer(registry);
}

void SetTargetSystem::updateHasTarget(entt::registry& registry) {
    // 筛选条件：敌我双方所有攻击型角色（排除治疗者，治疗者是另外逻辑）
    auto view = registry.view<TransformComponent, TargetComponent, StatsComponent>(entt::exclude<HealerTag>);
    for (auto [entity, transform, target, stats] : view.each()) {
        // 检查目标是否还有效
        if (!registry.valid(target.entity_)) {
            // 如果目标实体无效，则清除目标
            registry.remove<TargetComponent>(entity);
            spdlog::info("ID: {}, 目标: ID: {}, 无效, 清除目标", entt::to_integral(entity),
                         entt::to_integral(target.entity_));
            continue;
        }
        // 检查目标是否还在攻击范围之内（检测半径 = 角色攻击范围 + 目标角色半径）
        const auto& target_transform = registry.get<TransformComponent>(target.entity_);
        auto range_radius = stats.range_ + UNIT_RADIUS;
        if (distanceSquared(transform.position_, target_transform.position_) > range_radius * range_radius) {
            // 如果在攻击范围外，则清除目标
            registry.remove<TargetComponent>(entity);
            spdlog::info("ID: {}, 目标: ID: {}, 不在攻击范围之内, 清除目标", entt::to_integral(entity),
                         entt::to_integral(target.entity_));
            continue;
        }
    }
}

void SetTargetSystem::updateNoTargetPlayer(entt::registry& registry) {
    // 筛选条件：没有目标的玩家攻击型角色
    auto view_player_no_target = registry.view<TransformComponent, StatsComponent, PlayerComponent>(
        entt::exclude<TargetComponent, HealerTag>);
    auto view_enemy = registry.view<TransformComponent, EnemyComponent>();
    for (auto [player_entity, player_transform, player_stats, _] : view_player_no_target.each()) {
        auto range_radius = player_stats.range_ + UNIT_RADIUS;
        auto range_radius_sq = range_radius * range_radius;
        for (auto [enemy_entity, enemy_transform, _] : view_enemy.each()) {
            if (distanceSquared(player_transform.position_, enemy_transform.position_) <= range_radius_sq) {
                // 如果敌人在攻击范围之内，则设置目标
                registry.emplace<TargetComponent>(player_entity, enemy_entity);
                spdlog::info("玩家: ID: {}, 设置目标: ID: {}", entt::to_integral(player_entity),
                             entt::to_integral(enemy_entity));
                break;  // 设置一个目标敌人就停止检查
            }
        }
    }
}

void SetTargetSystem::updateNoTargetEnemy(entt::registry& registry) {
    // 筛选条件：没有目标的敌人角色（只考虑远程型，近战敌人的目标就是阻挡者）
    auto view_enemy_no_target = registry.view<TransformComponent, StatsComponent, EnemyComponent, RangedUnitTag>(
        entt::exclude<TargetComponent>);
    auto view_player = registry.view<TransformComponent, PlayerComponent>();
    for (auto [enemy_entity, enemy_transform, enemy_stats, _] : view_enemy_no_target.each()) {
        auto range_radius = enemy_stats.range_ + UNIT_RADIUS;
        auto range_radius_sq = range_radius * range_radius;
        for (auto [player_entity, player_transform, _] : view_player.each()) {
            if (distanceSquared(enemy_transform.position_, player_transform.position_) <= range_radius_sq) {
                // 如果玩家角色在攻击范围之内，则设置目标
                registry.emplace<TargetComponent>(enemy_entity, player_entity);
                spdlog::info("敌人: ID: {}, 设置目标: ID: {}", entt::to_integral(enemy_entity),
                             entt::to_integral(player_entity));
                break;  // 设置一个目标玩家角色就停止检查
            }
        }
    }
}

void SetTargetSystem::updateHealer(entt::registry& registry) {
    // --- 检查治疗者(玩家角色)的目标，选择血量百分比最低的受伤玩家角色作为目标 ---
    // 筛选条件：玩家治疗者角色
    auto view_healer = registry.view<HealerTag, TransformComponent, StatsComponent, PlayerComponent>();
    // 获取所有受伤玩家角色用于检测
    auto view_injured_player = registry.view<InjuredTag, TransformComponent, StatsComponent, PlayerComponent>();
    for (auto [healer_entity, healer_transform, healer_stats, _] : view_healer.each()) {
        float lowest_hp_percent = 1.0f;              // 保存最低血量百分比（初始为100%）
        entt::entity lowest_hp_player = entt::null;  // 保存最低血量百分比的玩家角色（初始为空）
        auto range_radius = healer_stats.range_ + UNIT_RADIUS;
        auto range_radius_sq = range_radius * range_radius;
        for (auto [player_entity, player_transform, player_stats, _] : view_injured_player.each()) {
            if (distanceSquared(healer_transform.position_, player_transform.position_) <= range_radius_sq) {
                auto hp_percent = player_stats.hp_ / player_stats.max_hp_;
                if (hp_percent < lowest_hp_percent) {
                    lowest_hp_percent = hp_percent;
                    lowest_hp_player = player_entity;
                }
            }
        }
        if (lowest_hp_player != entt::null) {
            // 如果找到了最低血量百分比的玩家角色，则设置目标
            registry.emplace_or_replace<TargetComponent>(healer_entity, lowest_hp_player);
        } else {
            // 否则移除目标(即使没有组件，也可以安全调用remove)
            registry.remove<TargetComponent>(healer_entity);
        }
    }
}

}  // namespace pyc::monster_war