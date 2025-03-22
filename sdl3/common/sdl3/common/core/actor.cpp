#include "sdl3/common/core/actor.h"

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

void Actor::move(std::chrono::duration<float> delta) {
    setPosition(
        glm::clamp(position_ + velocity_ * delta.count(), glm::vec2(), game_.getCurrentScene()->getWorldSize()));
}

}  // namespace sdl3
}  // namespace pyc
