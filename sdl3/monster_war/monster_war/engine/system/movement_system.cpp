module;

#include <chrono>

#include <entt/entity/registry.hpp>
#include <glm/vec2.hpp>

module monster_war.engine.system.movement_system;

import monster_war.engine.component.transform_component;
import monster_war.engine.component.velocity_component;

namespace pyc::monster_war {

void MovementSystem::update(entt::registry& registry, std::chrono::duration<float> delta_time) {
    auto view = registry.view<VelocityComponent, TransformComponent>();

    for (auto [_, velocity, transform] : view.each()) {
        transform.position_ += velocity.velocity_ * delta_time.count();
    }
}

}  // namespace pyc::monster_war
