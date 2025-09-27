#include "sunny_land/game/component/player_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/game/component/state/state_factory.h"

namespace pyc::sunny_land {

void PlayerComponent::init() {
    if (!owner_) {
        spdlog::error("PlayerComponent 没有所属游戏对象!");
        return;
    }

    // 获取必要的组件
    transform_component_ = owner_->getComponent<TransformComponent>();
    physics_component_ = owner_->getComponent<PhysicsComponent>();
    sprite_component_ = owner_->getComponent<SpriteComponent>();

    // 检查必要组件是否存在
    if (!transform_component_ || !physics_component_ || !sprite_component_) {
        spdlog::error("Player 对象缺少必要组件！");
    }

    // 初始化状态机
    current_state_ = StateFactory::create<IdleState>(this);
    if (current_state_) {
        setState(std::move(current_state_));
    }
    spdlog::debug("PlayerComponent 初始化完成。");
}

void PlayerComponent::handleInput(Context& context) {
    if (current_state_) {
        if (auto next_state = current_state_->handleInput(context)) {
            setState(std::move(next_state));
        }
    }
}

void PlayerComponent::update(std::chrono::duration<float> delta_time, Context& context) {
    if (current_state_) {
        if (auto next_state = current_state_->update(delta_time, context)) {
            setState(std::move(next_state));
        }
    }
}

void PlayerComponent::setState(std::unique_ptr<PlayerState> new_state) {
    if (!new_state) {
        spdlog::warn("尝试设置空的玩家状态!");
        return;
    }

    if (current_state_) {
        current_state_->exit();
    }

    current_state_ = std::move(new_state);
    spdlog::debug("玩家组件正在切换到状态: {}", typeid(*current_state_).name());
    current_state_->enter();
}

}  // namespace pyc::sunny_land