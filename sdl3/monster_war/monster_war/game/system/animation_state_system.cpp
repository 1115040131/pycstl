#include "monster_war/game/system/animation_state_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/utils/events.h"
#include "monster_war/game/component/blocked_by_component.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

using namespace entt::literals;

AnimationStateSystem::AnimationStateSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<AnimationFinishedEvent>().connect<&AnimationStateSystem::onAnimationFinishedEvent>(this);
}

AnimationStateSystem::~AnimationStateSystem() { dispatcher_.disconnect(this); }

void AnimationStateSystem::onAnimationFinishedEvent(const AnimationFinishedEvent& event) {
    if (!registry_.valid(event.entity_)) {
        return;
    }

    // 敌人动画结束逻辑
    if (registry_.all_of<EnemyComponent>(event.entity_)) {
        // 如果敌人被阻挡，则返回idle动画
        if (registry_.try_get<BlockedByComponent>(event.entity_)) {
            dispatcher_.enqueue(PlayAnimationEvent{event.entity_, "idle"_hs, true});
            spdlog::info("敌人行动动画结束, 返回idle动画, ID: {}", entt::to_integral(event.entity_));
        } else {
            dispatcher_.enqueue(PlayAnimationEvent{event.entity_, "walk"_hs, true});
            spdlog::info("敌人行动动画结束, 没有BlockedBy组件, 返回walk动画, ID: {}",
                         entt::to_integral(event.entity_));
        }
        // 移除动作锁定（硬直）标签
        registry_.remove<ActionLockTag>(event.entity_);
    } else if (registry_.all_of<PlayerComponent>(event.entity_)) {
        // 玩家动画结束，直接返回idle动画
        dispatcher_.enqueue(PlayAnimationEvent{event.entity_, "idle"_hs, true});
        spdlog::info("玩家动画结束, 返回idle动画, ID: {}", entt::to_integral(event.entity_));
    } else if (registry_.all_of<OneShotRemoveTag>(event.entity_)) {
        // 如果是一次性动画实体（例如死亡特效），则标记死亡待移除
        registry_.emplace_or_replace<DeadTag>(event.entity_);
    }
}

}  // namespace pyc::monster_war