#include "sdl3/common/core/actor.h"

#include "sdl3/common/core/scene.h"
#include "sdl3/common/raw/stats.h"

namespace pyc {
namespace sdl3 {

void Actor::move(std::chrono::duration<float> delta) {
    setPosition(
        glm::clamp(position_ + velocity_ * delta.count(), glm::vec2(), game_.getCurrentScene()->getWorldSize()));
}

void Actor::takeDamage(double damage) {
    if (stats_) {
        stats_->takeDamage(damage);
    }
}

bool Actor::isAlive() const {
    if (stats_) {
        return stats_->isAlive();
    }
    return true;
}

}  // namespace sdl3
}  // namespace pyc
