module;

#include <entt/entity/registry.hpp>

module monster_war.engine.system.ysort_system;

import monster_war.engine.component.render_component;
import monster_war.engine.component.transform_component;

namespace pyc::monster_war {

void YSortSystem::update(entt::registry& registry) {
    auto view = registry.view<TransformComponent, RenderComponent>();
    for (auto [_, transform, render] : view.each()) {
        render.depth = transform.position_.y;
    }
}

}  // namespace pyc::monster_war
