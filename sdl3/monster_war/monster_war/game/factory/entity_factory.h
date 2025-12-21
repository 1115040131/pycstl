#pragma once

#include <entt/entity/fwd.hpp>
#include <nlohmann/json_fwd.hpp>

#include "monster_war/game/data/entity_blueprint.h"

namespace pyc::monster_war {

class BlueprintManager;

/**
 * @brief 实体工厂，用于创建不同类型的实体
 *
 * 实体工厂通过蓝图管理器获取蓝图数据，并创建不同类型的实体。
 */
class EntityFactory {
public:
    /// @brief 实体工厂构造函数, 需要传入注册表和蓝图管理器。通过蓝图数据创建不同实体
    EntityFactory(entt::registry& registry, BlueprintManager& blueprint_manager);

    entt::entity createEnemyUnit(entt::id_type class_id, const glm::vec2& position, int target_waypoint_id,
                                 int level = 1, int rarity = 1);

private:
    // --- 组件创建函数 ---
    void addTransformComponent(entt::entity entity, const glm::vec2& position,
                               const glm::vec2& scale = glm::vec2(1.0f), float rotation = 0.0f);
    void addSpriteComponent(entt::entity entity, const SpriteBlueprint& sprite, const bool is_flipped = false);
    void addAnimationComponent(entt::entity entity,
                               const std::unordered_map<entt::id_type, AnimationBlueprint>& animation_blueprints,
                               const SpriteBlueprint& sprite_blueprint, entt::id_type default_animation_id);
    void addStatsComponent(entt::entity entity, const StatsBlueprint& stats, int level = 1, int rarity = 1);
    void addEnemyComponent(entt::entity entity, const EnemyBlueprint& enemy, int target_waypoint_id);
    void addAudioComponent(entt::entity entity, const SoundBlueprint& sounds);

private:
    entt::registry& registry_;
    BlueprintManager& blueprint_manager_;
};

}  // namespace pyc::monster_war