#include "monster_war/engine/system/movement_system.h"

#include <entt/entity/registry.hpp>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/component/velocity_component.h"

namespace pyc::monster_war {

void MovementSystem::update(entt::registry& registry, std::chrono::duration<float> delta_time) {
    auto view = registry.view<VelocityComponent, TransformComponent>();

    for (auto [_, velocity, transform] : view.each()) {
        transform.position_ += velocity.velocity_ * delta_time.count();
    }
}

}  // namespace pyc::monster_war