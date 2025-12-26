#include "monster_war/game/factory/entity_factory.h"

#include <entt/entity/registry.hpp>

#include "monster_war/engine/component/animation_component.h"
#include "monster_war/engine/component/audio_component.h"
#include "monster_war/engine/component/render_component.h"
#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/component/velocity_component.h"
#include "monster_war/game/component/blocker_component.h"
#include "monster_war/game/component/class_name_component.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/def/tag.h"
#include "monster_war/game/factory/blueprint_manager.h"

namespace pyc::monster_war {

using namespace entt::literals;

EntityFactory::EntityFactory(entt::registry& registry, BlueprintManager& blueprint_manager)
    : registry_(registry), blueprint_manager_(blueprint_manager) {}

entt::entity EntityFactory::createPlayerUnit(entt::id_type class_id, const glm::vec2& position, int level,
                                             int rarity) {
    auto entity = registry_.create();
    const auto& blueprint = blueprint_manager_.getPlayerClassBlueprint(class_id);

    // --- 添加组件 ---
    // 添加Transform组件
    addTransformComponent(entity, position);

    // 添加Sprite组件
    addSpriteComponent(entity, blueprint.sprite_);

    // 添加Animation组件 (默认动画为“walk”)
    addAnimationComponent(entity, blueprint.animations_, blueprint.sprite_, "idle"_hs);

    // 添加Audio组件
    addAudioComponent(entity, blueprint.sounds_);

    // 添加Stats组件
    addStatsComponent(entity, blueprint.stats_, level, rarity);

    // 添加Player组件
    addPlayerComponent(entity, blueprint.player_, rarity);

    // 补充其他必要组件
    registry_.emplace<ClassNameComponent>(entity, class_id, blueprint.display_info_.name_);
    registry_.emplace<RenderComponent>(entity);  // 使用默认主图层

    // 未来可添加其它组件

    return entity;
}

entt::entity EntityFactory::createEnemyUnit(entt::id_type class_id, const glm::vec2& position,
                                            int target_waypoint_id, int level, int rarity) {
    auto entity = registry_.create();
    const auto& blueprint = blueprint_manager_.getEnemyClassBlueprint(class_id);

    // --- 添加组件 ---
    // 添加Transform组件
    addTransformComponent(entity, position);

    // 添加Sprite组件
    addSpriteComponent(entity, blueprint.sprite_);

    // 添加Animation组件 (默认动画为“walk”)
    addAnimationComponent(entity, blueprint.animations_, blueprint.sprite_, "walk"_hs);

    // 添加Audio组件
    addAudioComponent(entity, blueprint.sounds_);

    // 添加Stats组件
    addStatsComponent(entity, blueprint.stats_, level, rarity);

    // 添加Enemy组件
    addEnemyComponent(entity, blueprint.enemy_, target_waypoint_id);

    // 补充其他必要组件
    registry_.emplace<ClassNameComponent>(entity, class_id, blueprint.display_info_.name_);
    registry_.emplace<RenderComponent>(entity);  // 使用默认主图层

    // 未来可添加其它组件

    return entity;
}

void EntityFactory::addTransformComponent(entt::entity entity, const glm::vec2& position, const glm::vec2& scale,
                                          float rotation) {
    registry_.emplace<TransformComponent>(entity, position, scale, rotation);
}

void EntityFactory::addSpriteComponent(entt::entity entity, const SpriteBlueprint& sprite, const bool is_flipped) {
    registry_.emplace<SpriteComponent>(entity,
                                       Sprite{
                                           sprite.path_,
                                           sprite.src_rect_,
                                           is_flipped,
                                       },
                                       sprite.size_, sprite.offset_);
    // 如果图片朝左就添加FaceLeftTag
    if (!sprite.face_right_) {
        registry_.emplace<FaceLeftTag>(entity);
    }
}

void EntityFactory::addAnimationComponent(
    entt::entity entity, const std::unordered_map<entt::id_type, AnimationBlueprint>& animation_blueprints,
    const SpriteBlueprint& sprite_blueprint, entt::id_type default_animation_id) {
    std::unordered_map<entt::id_type, Animation> animations;
    for (const auto& [anim_id, anim_blueprint] : animation_blueprints) {
        std::vector<AnimationFrame> frames;
        for (const auto& frame_blueprint : anim_blueprint.frames_) {
            Rect src_rect = sprite_blueprint.src_rect_;
            src_rect.position.x += frame_blueprint * src_rect.size.x;
            src_rect.position.y += anim_blueprint.row_ * src_rect.size.y;
            frames.emplace_back(src_rect, anim_blueprint.per_frame_);
        }
        animations.emplace(anim_id, std::move(frames));
    }
    registry_.emplace<AnimationComponent>(entity, std::move(animations), default_animation_id);
}

void EntityFactory::addStatsComponent(entt::entity entity, const StatsBlueprint& stats, int level, int rarity) {
    // 计算等级和稀有度对属性的影响 (未来可改成数据驱动方便调整)
    auto hp = statModify(stats.hp_, level, rarity);
    auto atk = statModify(stats.atk_, level, rarity);
    auto def = statModify(stats.def_, level, rarity);

    registry_.emplace<StatsComponent>(entity, hp, hp, atk, def, stats.range_, stats.atk_interval_,
                                      std::chrono::duration<float>::zero(), level, rarity);
}

void EntityFactory::addPlayerComponent(entt::entity entity, const PlayerBlueprint& player, int rarity) {
    auto cost = static_cast<int>(std::round(player.cost_ * (0.9f + 0.1f * rarity)));
    registry_.emplace<PlayerComponent>(entity, cost);
    // 添加类型标签(近战、远程、治疗)
    if (player.type_ == PlayerType::MELEE) {
        registry_.emplace<MeleeUnitTag>(entity);  // 近战单位标签
        // 近战类型添加阻挡者组件
        registry_.emplace<BlockerComponent>(entity, player.block_);
    } else if (player.type_ == PlayerType::RANGED) {
        registry_.emplace<RangedUnitTag>(entity);  // 远程单位标签
    }
    if (player.healer_) {
        registry_.emplace<HealerTag>(entity);  // 治疗单位标签
    }
}

void EntityFactory::addEnemyComponent(entt::entity entity, const EnemyBlueprint& enemy, int target_waypoint_id) {
    registry_.emplace<EnemyComponent>(entity, target_waypoint_id, enemy.speed_);
    registry_.emplace<VelocityComponent>(entity, glm::vec2{0.0f, 0.0f});
    if (enemy.ranged_) {
        registry_.emplace<RangedUnitTag>(entity);
    } else {
        registry_.emplace<MeleeUnitTag>(entity);
    }
}

void EntityFactory::addAudioComponent(entt::entity entity, const SoundBlueprint& sounds) {
    if (sounds.sounds_.empty()) {
        return;
    }
    registry_.emplace<AudioComponent>(entity, sounds.sounds_);
}

}  // namespace pyc::monster_war