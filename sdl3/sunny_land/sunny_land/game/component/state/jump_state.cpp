#include "sunny_land/game/component/state/jump_state.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/game/component/player_component.h"
#include "sunny_land/game/component/state/state_factory.h"

namespace pyc::sunny_land {

void JumpState::enter() {
    playAnimation("jump");
    // 设置跳跃初始速度
    auto physics_component = player_component_->getPhysicsComponent();
    physics_component->setVelocityY(-player_component_->getJumpVelocity());
    spdlog::debug("PlayerComponent 进入 JumpState，设置初始垂直速度为: {}", physics_component->getVelocity().y);
}

std::unique_ptr<PlayerState> JumpState::handleInput(Context& context) {
    const auto& input_manager = context.getInputManager();
    auto physics_component = player_component_->getPhysicsComponent();
    auto sprite_component = player_component_->getSpriteComponent();

    // 如果按下上下键，且与梯子重合，则切换到 ClimbState
    if (physics_component->hasCollidedLadder() &&
        (input_manager.isActionDown("move_up") || input_manager.isActionDown("move_down"))) {
        return StateFactory::create<ClimbState>(player_component_);
    }

    if (input_manager.isActionDown("move_left")) {
        if (physics_component->getVelocity().x > 0.0f) {
            physics_component->setVelocityX(0.0f);  // 如果当前速度是向右的，则先减速到0 (增强操控手感)
        }
        // 添加向左的水平力
        physics_component->addForce({-player_component_->getMoveForce(), 0.0f});
        sprite_component->setFlipped(true);  // 向左移动时翻转
    } else if (input_manager.isActionDown("move_right")) {
        if (physics_component->getVelocity().x < 0.0f) {
            physics_component->setVelocityX(0.0f);  // 如果当前速度是向左的，则先减速到0
        }
        // 添加向右的水平力
        physics_component->addForce({player_component_->getMoveForce(), 0.0f});
        sprite_component->setFlipped(false);  // 向右移动时不翻转
    }
    return nullptr;
}

std::unique_ptr<PlayerState> JumpState::update(std::chrono::duration<float>, Context&) {
    // 限制最大速度(水平方向)
    auto physics_component = player_component_->getPhysicsComponent();
    auto max_speed = player_component_->getMaxSpeed();
    physics_component->setVelocityX(glm::clamp(physics_component->getVelocity().x, -max_speed, max_speed));

    // 如果速度为正, 切换到 FallState
    if (physics_component->getVelocity().y >= 0.0f) {
        return StateFactory::create<FallState>(player_component_);
    }

    return nullptr;
}

}  // namespace pyc::sunny_land