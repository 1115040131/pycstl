#include "ghost_escape/player.h"

#include "sdl3/common/core/scene.h"
namespace pyc {
namespace sdl3 {

void Player::init() {
    Actor::init();
    max_speed_ = 500.F;
    sprite_idle_ = SpriteAnim::Create(this, ASSET("sprite/ghost-idle.png"), 2.F);
    sprite_move_ = SpriteAnim::Create(this, ASSET("sprite/ghost-move.png"), 2.F);
    sprite_move_->setActive(false);
}

void Player::clean() { Actor::clean(); }

void Player::handleEvents(SDL_Event& event) { Actor::handleEvents(event); }

void Player::update(std::chrono::duration<float> delta) {
    Actor::update(delta);
    keyboardControl();
    move(delta);
    syncCamera();
    checkState();
}

void Player::render() { Actor::render(); }

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

void Player::move(std::chrono::duration<float> delta) {
    setPosition(glm::clamp(position_ + velocity_ * delta.count(), glm::vec2(),
                           game_.getCurrentScene()->getWorldSize() - 20.F));
}

void Player::syncCamera() { game_.getCurrentScene()->setCameraPosition(position_ - game_.getScreenSize() / 2.F); }

void Player::checkState() {
    if (velocity_.x < 0) {
        sprite_idle_->setFlip(true);
        sprite_move_->setFlip(true);
    } else if (velocity_.x > 0) {
        sprite_idle_->setFlip(false);
        sprite_move_->setFlip(false);
    }

    auto curr_is_moving = glm::length(velocity_) > 50.F;
    if (is_moving_ != curr_is_moving) {
        is_moving_ = curr_is_moving;
        if (is_moving_) {
            sprite_idle_->setActive(false);
            sprite_move_->setActive(true);
            sprite_move_->syncFrame(*sprite_idle_);
        } else {
            sprite_idle_->setActive(true);
            sprite_move_->setActive(false);
            sprite_idle_->syncFrame(*sprite_move_);
        }
    }
}

}  // namespace sdl3
}  // namespace pyc