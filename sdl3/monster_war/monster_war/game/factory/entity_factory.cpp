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
#include "monster_war/game/component/projectile_component.h"
#include "monster_war/game/component/skill_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/component/unit_prep_component.h"
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

    // 添加ProjectileID组件
    addProjectileIDComponent(entity, blueprint.projectile_id_);

    // 添加Skill组件
    addSkillComponent(entity, blueprint.player_.skill_id_);

    // 补充其他必要组件
    registry_.emplace<ClassNameComponent>(entity, class_id, blueprint.display_info_.name_);
    registry_.emplace<RenderComponent>(entity);  // 使用默认主图层
    registry_.emplace<HasHealthBarTag>(entity);

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

    // 添加ProjectileID组件
    addProjectileIDComponent(entity, blueprint.projectile_id_);

    // 补充其他必要组件
    registry_.emplace<ClassNameComponent>(entity, class_id, blueprint.display_info_.name_);
    registry_.emplace<RenderComponent>(entity);  // 使用默认主图层
    registry_.emplace<HasHealthBarTag>(entity);

    return entity;
}

entt::entity EntityFactory::createProjectile(entt::id_type id, const glm::vec2& start_position,
                                             const glm::vec2& target_position, entt::entity target, float damage) {
    auto entity = registry_.create();
    const auto& blueprint = blueprint_manager_.getProjectileBlueprint(id);

    // --- 添加组件 ---
    // 添加ProjectileComponent
    registry_.emplace<ProjectileComponent>(entity, target, damage, start_position, target_position, start_position,
                                           blueprint.arc_height_, blueprint.total_flight_time_,
                                           std::chrono::duration<float>::zero());

    // 添加SpriteComponent
    addSpriteComponent(entity, blueprint.sprite_);

    // 添加TransformComponent
    addTransformComponent(entity, start_position);

    // 添加AudioComponent
    addAudioComponent(entity, blueprint.sounds_);

    // 添加RenderComponent(让投射物位于主图层+1，即可以遮住角色)
    registry_.emplace<RenderComponent>(entity, RenderComponent::kMainLayer + 1);
    return entity;
}

entt::entity EntityFactory::createUnitPrep(entt::id_type name_id, entt::id_type class_id, int cost,
                                           const glm::vec2& position) {
    auto entity = registry_.create();
    const auto& blueprint = blueprint_manager_.getPlayerClassBlueprint(class_id);

    // --- 添加组件 ---
    // 添加SpriteComponent
    addSpriteComponent(entity, blueprint.sprite_);

    // 添加TransformComponent
    addTransformComponent(entity, position);

    // 直接添加UnitPrepComponent组件
    registry_.emplace<UnitPrepComponent>(entity, name_id, blueprint.player_.type_, blueprint.stats_.range_, cost);

    // 补充渲染组件与显示攻击范围标志
    registry_.emplace<RenderComponent>(entity, 100);  // 显示优先度很高
    if (blueprint.player_.type_ == PlayerType::RANGED) {
        registry_.emplace<ShowRangeTag>(entity);
    }
    return entity;
}

entt::entity EntityFactory::createEnemyDeadEffect(entt::id_type class_id, const glm::vec2& position,
                                                  const bool is_flipped) {
    auto entity = registry_.create();
    const auto& blueprint = blueprint_manager_.getEnemyClassBlueprint(class_id);

    // --- 添加组件 ---
    // 添加Transform组件
    addTransformComponent(entity, position);

    // 添加Sprite组件
    addSpriteComponent(entity, blueprint.sprite_, is_flipped);

    // 添加Animation组件(死亡动画名称为“damage”)
    addOneAnimationComponent(entity, blueprint.animations_.at("damage"_hs), blueprint.sprite_, "damage"_hs);

    // 补充其他必要组件
    registry_.emplace<RenderComponent>(entity);
    registry_.emplace<OneShotRemoveTag>(entity);
    return entity;
}

entt::entity EntityFactory::createEffect(entt::id_type effect_id, const glm::vec2& position,
                                         const bool is_flipped) {
    auto entity = registry_.create();
    const auto& blueprint = blueprint_manager_.getEffectBlueprint(effect_id);

    // --- 添加组件 ---
    // 添加Transform组件
    addTransformComponent(entity, position);

    // 添加Sprite组件
    addSpriteComponent(entity, blueprint.sprite_, is_flipped);

    // 添加Animation组件, 只有一个动画，名称为特效id
    addOneAnimationComponent(entity, blueprint.animation_, blueprint.sprite_, effect_id);

    // 补充其他必要组件
    registry_.emplace<RenderComponent>(entity, RenderComponent::kMainLayer + 10);
    registry_.emplace<OneShotRemoveTag>(entity);
    return entity;
}

entt::entity EntityFactory::createSkillDisplay(entt::id_type effect_id, const glm::vec2& position) {
    auto entity = registry_.create();
    const auto& effect_blueprint = blueprint_manager_.getEffectBlueprint(effect_id);

    // --- 添加组件 ---
    // 添加Transform组件
    addTransformComponent(entity, position);

    // 添加Sprite组件
    addSpriteComponent(entity, effect_blueprint.sprite_);

    // 添加Animation组件 (角色上方的技能标识，循环播放)
    addOneAnimationComponent(entity, effect_blueprint.animation_, effect_blueprint.sprite_, effect_id, true);

    // 补充其他必要组件
    registry_.emplace<RenderComponent>(entity, RenderComponent::kMainLayer + 20);
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
        animations.emplace(anim_id, Animation{std::move(frames), anim_blueprint.events_});
    }
    registry_.emplace<AnimationComponent>(entity, std::move(animations), default_animation_id);
}

void EntityFactory::addOneAnimationComponent(entt::entity entity, const AnimationBlueprint& animation_blueprint,
                                             const SpriteBlueprint& sprite_blueprint, entt::id_type animation_id,
                                             bool loop) {
    std::vector<AnimationFrame> frames;
    for (const auto& frame_blueprint : animation_blueprint.frames_) {
        Rect src_rect = sprite_blueprint.src_rect_;
        src_rect.position.x += frame_blueprint * src_rect.size.x;
        src_rect.position.y += animation_blueprint.row_ * src_rect.size.y;
        frames.emplace_back(src_rect, animation_blueprint.per_frame_);
    }
    std::unordered_map<entt::id_type, Animation> animations = {
        {animation_id, Animation{std::move(frames), animation_blueprint.events_, loop}}};
    registry_.emplace<AnimationComponent>(entity, std::move(animations), animation_id);
}

void EntityFactory::addStatsComponent(entt::entity entity, const StatsBlueprint& stats, int level, int rarity) {
    // 计算等级和稀有度对属性的影响 (未来可改成数据驱动方便调整)
    auto hp = statModify(stats.hp_, level, rarity);
    auto atk = statModify(stats.atk_, level, rarity);
    auto def = statModify(stats.def_, level, rarity);

    registry_.emplace<StatsComponent>(entity, hp, hp, atk, def, stats.range_, stats.atk_interval_,
                                      stats.atk_interval_, level, rarity);
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
    if (!sounds.sounds_.empty()) {
        registry_.emplace<AudioComponent>(entity, sounds.sounds_);
    }
}

void EntityFactory::addProjectileIDComponent(entt::entity entity, entt::id_type id) {
    if (id != entt::null) {
        registry_.emplace<ProjectileIDComponent>(entity, id);
    }
}

void EntityFactory::addSkillComponent(entt::entity entity, entt::id_type skill_id) {
    const auto& skill = blueprint_manager_.getSkillBlueprint(skill_id);
    registry_.emplace<SkillComponent>(entity, skill_id, entt::null, skill.name_, skill.description_,
                                      skill.cooldown_, skill.duration_,
                                      skill.cooldown_ / 2.0f,  // 初始技能冷却时间为技能冷却时间的一半
                                      std::chrono::duration<float>::zero());
    // 如果是被动技能，则添加PassiveSkillTag与SkillReadyTag
    if (skill.passive_) {
        registry_.emplace<PassiveSkillTag>(entity);
        registry_.emplace<SkillReadyTag>(entity);
    }
}

}  // namespace pyc::monster_war