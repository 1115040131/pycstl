#include "sdl3/common/raw/stats.h"

namespace pyc {
namespace sdl3 {

Stats* Stats::CreateAndSet(Actor* parent, double max_health, double max_mana, double health_regen,
                           double mana_regon, double damage) {
    auto stats = std::make_unique<Stats>();
    stats->init();
    stats->health_.value = max_health;
    stats->health_.max_value = max_health;
    stats->health_.regen = health_regen;
    stats->mana_.value = max_mana;
    stats->mana_.max_value = max_mana;
    stats->mana_.regen = mana_regon;
    stats->damage_ = damage;
    return static_cast<Stats*>(parent->addChild(std::move(stats)));
}

void Stats::update(std::chrono::duration<float> delta) {
    Object::update(delta);
    health_.update(delta);
    mana_.update(delta);
    invincibleUpdate(delta);
}

Actor* Stats::getParent() const { return static_cast<Actor*>(parent_); }

void Stats::setParent(Object* parent) {
    if (!dynamic_cast<Actor*>(parent)) {
        fmt::println("Stats::setParent: parent is not an Actor");
    }
    parent_ = parent;
}

void Stats::takeDamage(double damage) {
    if (is_alive_ && !is_invincible_) {
        health_.value = std::max(0.0, health_.value - damage);

        if (health_.value <= 0.0) {
            is_alive_ = false;
        } else {
            is_invincible_ = true;
            invincible_time_counter_ = std::chrono::duration<float>::zero();
        }

        fmt::println("Player health: {}, is alive: {}", getHealth(), isAlive());
    }
}

void Stats::invincibleUpdate(std::chrono::duration<float> delta) {
    if (is_invincible_) {
        invincible_time_counter_ += delta;
        if (invincible_time_counter_ > invincible_time_) {
            is_invincible_ = false;
        }
    }
}

}  // namespace sdl3
}  // namespace pyc