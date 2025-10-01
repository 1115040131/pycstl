#include "sunny_land/game/component/state/walk_state.h"

#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/game/component/player_component.h"
#include "sunny_land/game/component/state/state_factory.h"

namespace pyc::sunny_land {

void WalkState::enter() { playAnimation("walk"); }

std::unique_ptr<PlayerState> WalkState::handleInput(Context& context) {
    const auto& input_manager = context.getInputManager();
    auto physics_component = player_component_->getPhysicsComponent();
    auto sprite_component = player_component_->getSpriteComponent();

    // 如果按下“jump”则切换到 JumpState
    if (input_manager.isActionPressed("jump")) {
        return StateFactory::create<JumpState>(player_component_);
    }  // 步行状态可以左右移动
    else if (input_manager.isActionDown("move_left")) {
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
    } else {
        // 如果没有按下左右移动键，则切换到 IdleState
        return StateFactory::create<IdleState>(player_component_);
    }
    return nullptr;
}

std::unique_ptr<PlayerState> WalkState::update(std::chrono::duration<float>, Context&) {
    auto physics_component = player_component_->getPhysicsComponent();
    auto max_speed = player_component_->getMaxSpeed();
    physics_component->setVelocityX(glm::clamp(physics_component->getVelocity().x, -max_speed, max_speed));

    // 如果离地，则切换到 FallState
    if (!physics_component->hasCollidedBelow()) {
        return StateFactory::create<FallState>(player_component_);
    }

    return nullptr;
}

}  // namespace pyc::sunny_land