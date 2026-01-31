#include "monster_war/game/system/effect_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/game/def/events.h"
#include "monster_war/game/factory/entity_factory.h"

namespace pyc::monster_war {

EffectSystem::EffectSystem(entt::registry& registry, entt::dispatcher& dispatcher, EntityFactory& entity_factory)
    : registry_(registry), dispatcher_(dispatcher), entity_factory_(entity_factory) {
    dispatcher_.sink<EnemyDeadEffectEvent>().connect<&EffectSystem::onEnemyDeadEffectEvent>(this);
    dispatcher_.sink<EffectEvent>().connect<&EffectSystem::onEffectEvent>(this);
}

EffectSystem::~EffectSystem() { dispatcher_.disconnect(this); }

void EffectSystem::onEnemyDeadEffectEvent(const EnemyDeadEffectEvent& event) {
    entity_factory_.createEnemyDeadEffect(event.class_id_, event.position_, event.is_flipped_);
}

void EffectSystem::onEffectEvent(const EffectEvent& event) {
    entity_factory_.createEffect(event.name_id_, event.position_, event.is_flipped_);
}

}  // namespace pyc::monster_war