#include "monster_war/game/system/timer_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include "monster_war/game/component/skill_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

TimerSystem::TimerSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {}

void TimerSystem::update(std::chrono::duration<float> delta_time) {
    updateAttackTimer(delta_time);
    updateSkillCooldownTimer(delta_time);
    updateSkillDurationTimer(delta_time);
}

void TimerSystem::updateAttackTimer(std::chrono::duration<float> delta_time) {
    auto view = registry_.view<StatsComponent>(entt::exclude<AttackReadyTag>);
    for (auto [entity, stats] : view.each()) {
        stats.atk_timer_ += delta_time;
        // 如果攻击计时器大于等于攻击间隔，代表冷却结束。添加“可攻击”标签，并重置攻击计时器
        if (stats.atk_timer_ >= stats.atk_interval_) {
            registry_.emplace_or_replace<AttackReadyTag>(entity);
            stats.atk_timer_ = {};
        }
    }
}

void TimerSystem::updateSkillCooldownTimer(std::chrono::duration<float> delta_time) {
    auto view = registry_.view<SkillComponent>(entt::exclude<SkillReadyTag, PassiveSkillTag>);
    for (auto [entity, skill] : view.each()) {
        skill.cooldown_timer_ += delta_time;
        // 如果技能冷却计时器大于等于技能冷却时间，代表冷却结束。添加“可施放”标签，并重置技能冷却计时器
        if (skill.cooldown_timer_ >= skill.cooldown_) {
            registry_.emplace_or_replace<SkillReadyTag>(entity);
            skill.cooldown_timer_ = {};
            dispatcher_.enqueue(SkillReadyEvent{entity});
        }
    }
}

void TimerSystem::updateSkillDurationTimer(std::chrono::duration<float> delta_time) {
    auto view = registry_.view<SkillComponent, SkillActiveTag>(entt::exclude<PassiveSkillTag>);
    for (auto [entity, skill] : view.each()) {
        skill.duration_timer_ += delta_time;
        // 如果技能冷却计时器大于等于技能冷却时间，代表冷却结束。添加“可施放”标签，并重置技能冷却计时器
        if (skill.duration_timer_ >= skill.duration_) {
            registry_.emplace_or_replace<SkillActiveTag>(entity);
            skill.duration_timer_ = {};
            dispatcher_.enqueue(SkillDurationEndEvent{entity});
        }
    }
}

}  // namespace pyc::monster_war