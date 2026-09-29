module;

#include <chrono>
#include <memory>

#include <SDL3/SDL_scancode.h>
#include <glm/glm.hpp>

module sunny_land.game.player;

import common.string_hash;
import sunny_land.engine.core;
import sunny_land.engine.input.input_manager;

namespace pyc::sunny_land {

void IdleState::enter() { playAnimation("idle"); }

std::unique_ptr<PlayerState> IdleState::handleInput(Context& context) {
    const auto& input_manager = context.getInputManager();
    auto physics_component = player_component_->getPhysicsComponent();

    // 如果按下上键，且与梯子重合，则切换到 ClimbState
    if (physics_component->hasCollidedLadder() && input_manager.isActionDown("move_up")) {
        return StateFactory::create<ClimbState>(player_component_);
    }

    // 如果按下“move_down”且在梯子顶层，则切换到 ClimbState
    if (physics_component->isOnTopLadder() && input_manager.isActionDown("move_down")) {
        // 需要向下移动一点，确保下一帧能与梯子碰撞（否则会切换回FallState）
        player_component_->getTransformComponent()->translate(glm::vec2(0, 2.0f));
        return StateFactory::create<ClimbState>(player_component_);
    }

    // 如果按下了左右移动键，则切换到 WalkState
    if (input_manager.isActionDown("move_left") || input_manager.isActionDown("move_right")) {
        return StateFactory::create<WalkState>(player_component_);
    }

    // 如果按下“jump”则切换到 JumpState
    if (input_manager.isActionPressed("jump")) {
        return StateFactory::create<JumpState>(player_component_);
    }
    return nullptr;
}

std::unique_ptr<PlayerState> IdleState::update(std::chrono::duration<float>, Context&) {
    // 应用摩擦力
    auto physics_component = player_component_->getPhysicsComponent();
    auto friction_factor = player_component_->getFrictionFactor();
    physics_component->setVelocityX(physics_component->getVelocity().x * friction_factor);

    // 如果离地，则切换到 FallState
    if (!player_component_->is_on_ground()) {
        return StateFactory::create<FallState>(player_component_);
    }

    return nullptr;
}

}  // namespace pyc::sunny_land
