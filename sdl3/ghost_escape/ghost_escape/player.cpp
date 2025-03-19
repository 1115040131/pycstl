#include "ghost_escape/player.h"

#include "sdl3/common/scene.h"

namespace pyc {
namespace sdl3 {

void Player::init() { max_speed_ = 500.F; }

void Player::clean() {}

void Player::handleEvents(SDL_Event&) {}

void Player::update(std::chrono::duration<float> delta) {
    keyboardControl();
    move(delta);
    syncCamera();
}

void Player::render() {
    game_.drawBoundary(render_position_, render_position_ + glm::vec2(20.F), 5.F, {1, 0, 0, 1});
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

void Player::move(std::chrono::duration<float> delta) {
    setPosition(glm::clamp(position_ + velocity_ * delta.count(), glm::vec2(),
                           game_.getCurrentScene()->getWorldSize() - 20.F));
}

void Player::syncCamera() { game_.getCurrentScene()->setCameraPosition(position_ - game_.getScreenSize() / 2.F); }

}  // namespace sdl3
}  // namespace pyc