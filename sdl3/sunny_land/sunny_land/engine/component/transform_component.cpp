#include "sunny_land/engine/component/transform_component.h"

#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/object/game_object.h"

namespace pyc::sunny_land {

void TransformComponent::setScale(glm::vec2 scale) {
    scale_ = std::move(scale);
    if (owner_) {
        auto sprite_component = owner_->getComponent<SpriteComponent>();
        if (sprite_component) {
            sprite_component->updateOffset();
        }
    }
}

}  // namespace pyc::sunny_land
