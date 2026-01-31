#include "monster_war/game/system/animation_event_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/utils/events.h"
#include "monster_war/game/component/blocked_by_component.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/component/projectile_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/component/target_component.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

using namespace entt::literals;

AnimationEventSystem::AnimationEventSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<AnimationEvent>().connect<&AnimationEventSystem::onAnimationEvent>(this);
}

AnimationEventSystem::~AnimationEventSystem() { dispatcher_.disconnect(this); }

void AnimationEventSystem::onAnimationEvent(const AnimationEvent& event) {
    if (!registry_.valid(event.entity_)) {
        return;
    }
    // 根据不同的事件id，调用不同的处理函数
    if (event.event_name_id_ == "hit"_hs) {
        handleHitEvent(event);
    } else if (event.event_name_id_ == "emit"_hs) {
        handleEmitEvent(event);
    }
}

void AnimationEventSystem::handleHitEvent(const AnimationEvent& event) {
    // 玩家命中事件：治疗或攻击当前目标
    if (registry_.all_of<PlayerComponent>(event.entity_)) {
        // 命中时有可能目标已经解锁，因此需要检查
        if (auto target_component = registry_.try_get<TargetComponent>(event.entity_)) {
            const auto& stats = registry_.get<StatsComponent>(event.entity_);
            if (registry_.all_of<HealerTag>(event.entity_)) {
                dispatcher_.enqueue(HealEvent{event.entity_, target_component->entity_, stats.atk_});
            } else {
                dispatcher_.enqueue(AttackEvent{event.entity_, target_component->entity_, stats.atk_});
            }
            // 播放“hit”音效
            dispatcher_.enqueue(PlaySoundEvent{event.entity_, "hit"_hs});
        }
    } else if (registry_.all_of<EnemyComponent>(event.entity_)) {
        // 命中时有可能目标（阻挡者）已经解锁，因此需要检查
        if (auto blocked_by = registry_.try_get<BlockedByComponent>(event.entity_)) {
            const auto& stats = registry_.get<StatsComponent>(event.entity_);
            dispatcher_.enqueue(AttackEvent{event.entity_, blocked_by->entity_, stats.atk_});
        }
    }
}

void AnimationEventSystem::handleEmitEvent(const AnimationEvent& event) {
    // 确认“目标组件”依然存在，且其中的实体也有效
    auto target = registry_.try_get<TargetComponent>(event.entity_);
    if (!target || !registry_.valid(target->entity_)) {
        return;
    }

    const auto [transform, stats, projectile_id] =
        registry_.get<TransformComponent, StatsComponent, ProjectileIDComponent>(event.entity_);

    // 发射投射物事件
    dispatcher_.enqueue(EmitProjectileEvent{
        projectile_id.id_,
        target->entity_,
        transform.position_,
        registry_.get<TransformComponent>(target->entity_).position_,
        stats.atk_,
    });

    // 播放“emit”音效
    dispatcher_.enqueue(PlaySoundEvent{event.entity_, "emit"_hs});
}

}  // namespace pyc::monster_war