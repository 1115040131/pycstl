#include "sunny_land/game/component/state/idle_state.h"

#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/game/component/player_component.h"
#include "sunny_land/game/component/state/state_factory.h"

namespace pyc::sunny_land {

void IdleState::enter() { playAnimation("idle"); }

std::unique_ptr<PlayerState> IdleState::handleInput(Context& context) {
    const auto& input_manager = context.getInputManager();

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
    if (!physics_component->hasCollidedBelow()) {
        return StateFactory::create<FallState>(player_component_);
    }

    return nullptr;
}

}  // namespace pyc::sunny_land