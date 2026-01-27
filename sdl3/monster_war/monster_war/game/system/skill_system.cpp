#include "monster_war/game/system/skill_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/utils/events.h"
#include "monster_war/game/component/cost_regen_component.h"
#include "monster_war/game/component/skill_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/def/constants.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/def/tag.h"
#include "monster_war/game/factory/blueprint_manager.h"
#include "monster_war/game/factory/entity_factory.h"

namespace pyc::monster_war {

using namespace entt::literals;

SkillSystem::SkillSystem(entt::registry& registry, entt::dispatcher& dispatcher, EntityFactory& entity_factory)
    : registry_(registry), dispatcher_(dispatcher), entity_factory_(entity_factory) {
    dispatcher_.sink<SkillReadyEvent>().connect<&SkillSystem::onSkillReadyEvent>(this);
    dispatcher_.sink<SkillActiveEvent>().connect<&SkillSystem::onSkillActiveEvent>(this);
    dispatcher_.sink<SkillDurationEndEvent>().connect<&SkillSystem::onSkillDurationEndEvent>(this);
    dispatcher_.sink<RemovePlayerUnitEvent>().connect<&SkillSystem::onRemoveUnitEvent>(this);
}

SkillSystem::~SkillSystem() { dispatcher_.disconnect(this); }

void SkillSystem::onSkillReadyEvent(const SkillReadyEvent& event) {
    if (event.entity_ == entt::null || !registry_.valid(event.entity_)) {
        return;
    }
    // 获取技能和位置组件
    const auto& transform = registry_.get<TransformComponent>(event.entity_);
    auto& skill = registry_.get<SkillComponent>(event.entity_);
    // 先删除可能存在的显示实体
    if (skill.display_entity_ != entt::null && registry_.valid(skill.display_entity_)) {
        registry_.emplace_or_replace<DeadTag>(skill.display_entity_);
    }
    // 创建新的显示实体 (技能准备就绪)
    skill.display_entity_ =
        entity_factory_.createSkillDisplay("skill_ready"_hs, transform.position_ + SKILL_DISPLAY_OFFSET);
}

void SkillSystem::onSkillActiveEvent(const SkillActiveEvent& event) {
    if (event.entity_ == entt::null || !registry_.valid(event.entity_)) {
        return;
    }
    // 如果技能未就绪，则返回
    if (!registry_.any_of<SkillReadyTag>(event.entity_)) {
        return;
    }

    // 获取技能和位置组件
    const auto& transform = registry_.get<TransformComponent>(event.entity_);
    auto& skill = registry_.get<SkillComponent>(event.entity_);
    // 先删除可能存在的显示实体
    if (skill.display_entity_ != entt::null && registry_.valid(skill.display_entity_)) {
        registry_.emplace_or_replace<DeadTag>(skill.display_entity_);
    }
    // 创建新的显示实体 (技能激活)
    skill.display_entity_ =
        entity_factory_.createSkillDisplay("skill_active"_hs, transform.position_ + SKILL_DISPLAY_OFFSET);

    // 移除技能准备标签,添加技能激活标签
    registry_.remove<SkillReadyTag>(event.entity_);
    registry_.emplace<SkillActiveTag>(event.entity_);

    // 如果技能是盾御，且动作未锁定，则播放guard动画
    if (skill.skill_id_ == "shield"_hs && !registry_.any_of<ActionLockTag>(event.entity_)) {
        dispatcher_.enqueue(PlayAnimationEvent{event.entity_, "idle"_hs, true});
    }

    // 添加Buff
    addBuff(event.entity_, skill.skill_id_);
}

void SkillSystem::onSkillDurationEndEvent(const SkillDurationEndEvent& event) {
    if (event.entity_ == entt::null || !registry_.valid(event.entity_)) {
        return;
    }

    auto& skill = registry_.get<SkillComponent>(event.entity_);
    // 先删除可能存在的显示实体
    if (skill.display_entity_ != entt::null && registry_.valid(skill.display_entity_)) {
        registry_.emplace_or_replace<DeadTag>(skill.display_entity_);
    }

    // 移除技能激活标签
    registry_.remove<SkillActiveTag>(event.entity_);

    // 如果技能是盾御，且动作未锁定，则播放idle动画
    if (skill.skill_id_ == "shield"_hs && !registry_.any_of<ActionLockTag>(event.entity_)) {
        dispatcher_.enqueue(PlayAnimationEvent{event.entity_, "idle"_hs, true});
    }

    // 移除Buff
    removeBuff(event.entity_, skill.skill_id_);
}

void SkillSystem::onRemoveUnitEvent(const RemovePlayerUnitEvent& event) {
    if (event.entity_ == entt::null || !registry_.valid(event.entity_)) {
        return;
    }
    // 移除技能显示实体
    if (auto skill = registry_.try_get<SkillComponent>(event.entity_)) {
        if (skill->display_entity_ != entt::null && registry_.valid(skill->display_entity_)) {
            registry_.emplace_or_replace<DeadTag>(skill->display_entity_);
        }
    }
}

void SkillSystem::addBuff(entt::entity entity, entt::id_type skill_id) {
    if (entity == entt::null || !registry_.valid(entity)) {
        return;
    }

    // 获取Buff信息
    auto blueprint_mgr = registry_.ctx().get<std::shared_ptr<BlueprintManager>>();
    const auto& buff_blueprint = blueprint_mgr->getSkillBlueprint(skill_id).buff_;

    // 将Buff应用到角色的Stats中
    auto& stats = registry_.get<StatsComponent>(entity);
    stats.hp_ *= buff_blueprint.hp_multiplier_;
    stats.atk_ *= buff_blueprint.atk_multiplier_;
    stats.def_ *= buff_blueprint.def_multiplier_;
    stats.range_ *= buff_blueprint.range_multiplier_;
    stats.atk_interval_ *= buff_blueprint.atk_interval_multiplier_;

    // 若存在Cost相关Buff，则添加COST恢复组件
    if (buff_blueprint.cost_regen_ > 0.0f) {
        registry_.emplace_or_replace<CostRegenComponent>(entity, buff_blueprint.cost_regen_);
    }
}

void SkillSystem::removeBuff(entt::entity entity, entt::id_type skill_id) {
    if (entity == entt::null || !registry_.valid(entity)) {
        return;
    }

    // 获取Buff信息
    auto blueprint_mgr = registry_.ctx().get<std::shared_ptr<BlueprintManager>>();
    const auto& buff_blueprint = blueprint_mgr->getSkillBlueprint(skill_id).buff_;

    // 从角色的Stats中移除Buff
    auto& stats = registry_.get<StatsComponent>(entity);
    stats.hp_ /= buff_blueprint.hp_multiplier_;
    stats.atk_ /= buff_blueprint.atk_multiplier_;
    stats.def_ /= buff_blueprint.def_multiplier_;
    stats.range_ /= buff_blueprint.range_multiplier_;
    stats.atk_interval_ /= buff_blueprint.atk_interval_multiplier_;

    // 若存在Cost相关Buff，则移除COST恢复组件
    if (buff_blueprint.cost_regen_ > 0.0f) {
        registry_.remove<CostRegenComponent>(entity);
    }
}

}  // namespace pyc::monster_war