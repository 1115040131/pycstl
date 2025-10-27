#include "sunny_land/game/component/state/climb_state.h"

#include "sunny_land/engine/component/animation_component.h"
#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/game/component/player_component.h"
#include "sunny_land/game/component/state/state_factory.h"

namespace pyc::sunny_land {

void ClimbState::enter() {
    playAnimation("climb");
    if (auto physics = player_component_->getPhysicsComponent()) {
        physics->setUseGravity(false);
    }
}

void ClimbState::exit() {
    if (auto physics = player_component_->getPhysicsComponent()) {
        physics->setUseGravity(true);
    }
}

std::unique_ptr<PlayerState> ClimbState::handleInput(Context& context) {
    const auto& input_manager = context.getInputManager();
    auto physics_component = player_component_->getPhysicsComponent();
    auto animation_component = player_component_->getAnimationComponent();

    // --- 攀爬状态下，按键则移动，不按键则静止 ---
    auto is_up = input_manager.isActionDown("move_up");
    auto is_down = input_manager.isActionDown("move_down");
    auto is_left = input_manager.isActionDown("move_left");
    auto is_right = input_manager.isActionDown("move_right");
    auto speed = player_component_->getClimbSpeed();

    physics_component->setVelocityY(is_up ? -speed : is_down ? speed : 0.0f);
    physics_component->setVelocityX(is_left ? -speed : is_right ? speed : 0.0f);

    // --- 根据是否有按键决定动画播放情况 ---
    if (is_up || is_down || is_left || is_right) {
        animation_component->resumeAnimation();  // 有按键则恢复动画播放
    } else {
        animation_component->stopAnimation();  // 无按键则停止动画播放
    }

    // 按跳跃键主动离开攀爬状态
    if (input_manager.isActionPressed("jump")) {
        return StateFactory::create<JumpState>(player_component_);
    }

    return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::update(std::chrono::duration<float>, Context&) {
    auto physics_component = player_component_->getPhysicsComponent();
    // 如果着地，则切换到 IdleState
    if (physics_component->hasCollidedBelow()) {
        return StateFactory::create<IdleState>(player_component_);
    }
    // 如果离开梯子区域，则切换到 FallState（能走到这里 说明非着地状态）
    if (!physics_component->hasCollidedLadder()) {
        return StateFactory::create<FallState>(player_component_);
    }
    return nullptr;
}

}  // namespace pyc::sunny_land