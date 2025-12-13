#include "monster_war/engine/system/render_system.h"

#include <entt/entity/registry.hpp>

#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/render/renderer.h"

namespace pyc::monster_war {

void RenderSystem::update(entt::registry& registry, Renderer& renderer, const Camera& camera) {
    auto view = registry.view<TransformComponent, SpriteComponent>();
    for (auto [_, transform, sprite] : view.each()) {
        auto position = transform.position_ + sprite.offset_;
        auto size = sprite.size_ * transform.scale_;
        renderer.drawSprite(camera, sprite.sprite_, position, size);
    }
}

}  // namespace pyc::monster_war
