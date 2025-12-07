#include "monster_war/engine/component/transform_component.h"

#include "monster_war/engine/component/collider_component.h"
#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/object/game_object.h"

namespace pyc::monster_war {

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

}  // namespace pyc::monster_war
