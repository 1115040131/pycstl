#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace pyc::monster_war {

class EntityFactory;

class SkillReadyEvent;
class SkillActiveEvent;
class SkillDurationEndEvent;
class RemovePlayerUnitEvent;

/**
 * @brief 技能系统
 * @note 用于管理技能的施放与显示、Buff增删等操作
 */
class SkillSystem {
public:
    SkillSystem(entt::registry& registry, entt::dispatcher& dispatcher, EntityFactory& entity_factory);
    ~SkillSystem();

private:
    // 事件回调函数
    void onSkillReadyEvent(const SkillReadyEvent& event);
    void onSkillActiveEvent(const SkillActiveEvent& event);
    void onSkillDurationEndEvent(const SkillDurationEndEvent& event);
    void onRemoveUnitEvent(const RemovePlayerUnitEvent& event);

    // Buff增删函数
    void addBuff(entt::entity entity, entt::id_type skill_id);
    void removeBuff(entt::entity entity, entt::id_type skill_id);

private:
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
    EntityFactory& entity_factory_;
};

}  // namespace pyc::monster_war