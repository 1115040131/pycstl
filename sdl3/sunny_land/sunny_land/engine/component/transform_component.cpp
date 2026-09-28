module;

#include <utility>

#include <SDL3/SDL_rect.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

module sunny_land.engine.core;

namespace pyc::sunny_land {

void TransformComponent::setScale(glm::vec2 scale) {
    scale_ = std::move(scale);
    if (owner_) {
        auto sprite_component = owner_->getComponent<SpriteComponent>();
        if (sprite_component) {
            sprite_component->updateOffset();
        }
        auto collider_component = owner_->getComponent<ColliderComponent>();
        if (collider_component) {
            collider_component->updateOffset();
        }
    }
}

}  // namespace pyc::sunny_land
