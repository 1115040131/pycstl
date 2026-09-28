module;

#include <chrono>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "ghost_escape/core/asset.h"
#include "ghost_escape/core/set_name.h"

module ghost_escape.player;

import ghost_escape.actor;
import ghost_escape.affiliate.sprite_anim;
import ghost_escape.core;
import ghost_escape.raw.timer;
import ghost_escape.weapon_thunder;
import ghost_escape.world.effect;

namespace pyc::sdl3 {

using namespace std::chrono_literals;

void Player::init() {
    Actor::init();

#ifdef DEBUG_MODE
    SET_NAME(Player);
#endif

    max_speed_ = 500.F;
    anim_idle_ = SpriteAnim::CreateAndSet(this, ASSET("sprite/ghost-idle.png"), 2.F);
    anim_move_ = SpriteAnim::CreateAndSet(this, ASSET("sprite/ghost-move.png"), 2.F);
    anim_move_->setActive(false);

    collider_ = Collider::CreateAndSet(this, anim_idle_->getSize() / 2.F);

    stats_ = Stats::CreateAndSet(this);

    effect_ = Effect::CreateAndSet(game_.getCurrentScene().get(), ASSET("effect/1764.png"), {}, 2.F);
    effect_->setActive(false);

    weapon_thunder_ = WeaponThunder::CreateAndSet(this, 2s, 40.F);

    flash_timer_ = Timer::CreateAndSet(this, 0.4s);
    flash_timer_->start();
}

void Player::clean() { Actor::clean(); }

void Player::update(std::chrono::duration<float> delta) {
    Actor::update(delta);
    keyboardControl();
    move(delta);
    syncCamera();
    checkState();
    checkIsDead();
}

void Player::render() {
    if (stats_->isInvincible() && flash_timer_->getProgress() < 0.5f) {
        return;
    }
    Actor::render();
}

void Player::takeDamage(double damage) {
    if (!stats_ || stats_->isInvincible()) {
        return;
    }
    Actor::takeDamage(damage);
    game_.playSound(ASSET("sound/hit-flesh-02-266309.mp3"));
}

void Player::keyboardControl() {
    // 玩家移动
    velocity_ *= 0.9F;
    auto keyboard_state = SDL_GetKeyboardState(nullptr);
    if (keyboard_state[SDL_SCANCODE_W]) {
        velocity_.y = -max_speed_;
    }
    if (keyboard_state[SDL_SCANCODE_S]) {
        velocity_.y = max_speed_;
    }
    if (keyboard_state[SDL_SCANCODE_A]) {
        velocity_.x = -max_speed_;
    }
    if (keyboard_state[SDL_SCANCODE_D]) {
        velocity_.x = max_speed_;
    }
}

void Player::syncCamera() { game_.getCurrentScene()->setCameraPosition(position_ - game_.getScreenSize() / 2.F); }

void Player::checkState() {
    if (velocity_.x < 0) {
        anim_idle_->setFlip(true);
        anim_move_->setFlip(true);
    } else if (velocity_.x > 0) {
        anim_idle_->setFlip(false);
        anim_move_->setFlip(false);
    }

    auto curr_is_moving = glm::length(velocity_) > 50.F;
    if (is_moving_ != curr_is_moving) {
        is_moving_ = curr_is_moving;
        if (is_moving_) {
            anim_idle_->setActive(false);
            anim_move_->setActive(true);
            anim_move_->syncFrame(*anim_idle_);
        } else {
            anim_idle_->setActive(true);
            anim_move_->setActive(false);
            anim_idle_->syncFrame(*anim_move_);
        }
    }
}

void Player::checkIsDead() {
    if (!stats_->isAlive()) {
        effect_->setPosition(getPosition());
        effect_->setActive(true);
        setActive(false);
        game_.playSound(ASSET("sound/female-scream-02-89290.mp3"));
    }
}

}  // namespace pyc::sdl3
