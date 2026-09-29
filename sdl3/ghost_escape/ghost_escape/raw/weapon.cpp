module;

#include <algorithm>
#include <chrono>
#include <memory>
#include <utility>

#include <fmt/base.h>
#include <glm/glm.hpp>

module ghost_escape.raw.weapon;

import ghost_escape.actor;
import ghost_escape.core;
import ghost_escape.world.spell;

namespace pyc::sdl3 {

void Weapon::update(std::chrono::duration<float> delta) {
    Object::update(delta);
    if (timer_ < cool_down_) {
        timer_ += delta;
    }
}

Actor* Weapon::getParent() const { return static_cast<Actor*>(parent_); }

void Weapon::setParent(Object* parent) {
    if (!dynamic_cast<Actor*>(parent)) {
        fmt::println("Weapon::setParent: parent is not an Actor");
    }
    parent_ = parent;
}

float Weapon::getCoolDownPercent() const { return std::clamp(timer_ / cool_down_, 0.0f, 1.0f); }

bool Weapon::canAttack() const { return timer_ >= cool_down_ && getParent()->getStats()->canUseMana(mana_cost_); }

void Weapon::attack(const glm::vec2& position, std::unique_ptr<Spell> spell) {
    if (!spell) {
        return;
    }
    getParent()->getStats()->useMana(mana_cost_);
    timer_ = std::chrono::duration<float>::zero();
    spell->setPosition(position);
    game_.getCurrentScene()->addChild(std::move(spell));
}

}  // namespace pyc::sdl3
