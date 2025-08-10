#include "ghost_escape/core/actor.h"

#include "ghost_escape/core/scene.h"

namespace pyc {
namespace sdl3 {

void Actor::update(std::chrono::duration<float> delta) {
    updateHealthBar();
    ObjectWorld::update(delta);
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

void Actor::move(std::chrono::duration<float> delta) {
    setPosition(
        glm::clamp(position_ + velocity_ * delta.count(), glm::vec2(), game_.getCurrentScene()->getWorldSize()));
}

void Actor::updateHealthBar() {
    if (stats_ && health_bar_) {
        health_bar_->setPercentage(stats_->getHealth() / stats_->getMaxHealth());
    }
}

}  // namespace sdl3
}  // namespace pyc
