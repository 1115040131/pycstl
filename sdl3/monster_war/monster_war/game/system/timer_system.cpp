#include "monster_war/game/system/timer_system.h"

#include <entt/entity/registry.hpp>

#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

void TimerSystem::update(entt::registry& registry, std::chrono::duration<float> delta_time) {
    updateAttackTimer(registry, delta_time);
}

void TimerSystem::updateAttackTimer(entt::registry& registry, std::chrono::duration<float> delta_time) {
    auto view = registry.view<StatsComponent>(entt::exclude<AttackReadyTag>);
    for (auto [entity, stats] : view.each()) {
        stats.atk_timer_ += delta_time;
        // 如果攻击计时器大于等于攻击间隔，代表冷却结束。添加“可攻击”标签，并重置攻击计时器
        if (stats.atk_timer_ >= stats.atk_interval_) {
            registry.emplace_or_replace<AttackReadyTag>(entity);
            stats.atk_timer_ = {};
        }
    }
}

}  // namespace pyc::monster_war