#include "ghost_escape/player.h"

#include "sdl3/common/affiliate/sprite_anim.h"
#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

void Player::init() {
    Actor::init();
    max_speed_ = 500.F;
    addChild(SpriteAnim::Create(this, ASSET("sprite/ghost-idle.png"), 2.F));
    // addChild(Sprite::Create(this, ASSET("sprite/ghost-idle.png")));
}

void Player::clean() { Actor::clean(); }

void Player::handleEvents(SDL_Event& event) { Actor::handleEvents(event); }

void Player::update(std::chrono::duration<float> delta) {
    Actor::update(delta);
    keyboardControl();
    move(delta);
    syncCamera();
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

}  // namespace sdl3
}  // namespace pyc