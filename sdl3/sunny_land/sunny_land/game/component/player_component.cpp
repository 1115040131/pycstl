#include "sunny_land/game/component/player_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/animation_component.h"
#include "sunny_land/engine/component/audio_component.h"
#include "sunny_land/engine/component/health_component.h"
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
    animation_component_ = owner_->getComponent<AnimationComponent>();
    audio_component_ = owner_->getComponent<AudioComponent>();
    health_component_ = owner_->getComponent<HealthComponent>();

    // 检查必要组件是否存在
    if (!transform_component_ || !physics_component_ || !sprite_component_ || !animation_component_ ||
        !audio_component_ || !health_component_) {
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
    if (!current_state_) {
        return;
    }

    // 一旦离地，开始计时 Coyote Timer
    if (!physics_component_->hasCollidedBelow()) {
        coyote_timer_ += delta_time;
    } else {  // 如果碰撞到地面，重置 Coyote Timer
        coyote_timer_ = {};
    }

    // 如果处于无敌状态，则进行闪烁
    if (health_component_->isInvincible()) {
        flash_timer_ += delta_time;  // 闪烁计时器增加
        if (flash_timer_ >= 2 * flash_interval_) {
            flash_timer_ -= 2 * flash_interval_;  // 闪烁计时器在 0～2倍闪烁间隔 中循环
        }
        // 一半时间可见，一半时间不可见。
        if (flash_timer_ < flash_interval_) {
            sprite_component_->setHidden(true);
        } else {
            sprite_component_->setHidden(false);
        }
    } else if (sprite_component_->isHidden()) {  // 非无敌状态时确保精灵可见
        sprite_component_->setHidden(false);
    }

    if (auto next_state = current_state_->update(delta_time, context)) {
        setState(std::move(next_state));
    }
}

bool PlayerComponent::takeDamage(int damage) {
    if (isDead() || !health_component_ || damage <= 0) {
        spdlog::warn("玩家已死亡或却少必要组件，并未造成伤害。");
        return false;
    }

    bool success = health_component_->takeDamage(damage);
    if (!success) {
        return false;
    }

    // --- 成功造成伤害了，根据是否存活决定状态切换
    if (health_component_->isAlive()) {
        spdlog::debug("玩家受到了 {} 点伤害，当前生命值: {}/{}。", damage, health_component_->getCurrentHealth(),
                      health_component_->getMaxHealth());
        // 切换到受伤状态
        setState(StateFactory::create<HurtState>(this));
    } else {
        spdlog::debug("玩家死亡。");
        is_dead_ = true;
        // 切换到死亡状态
        setState(StateFactory::create<DeadState>(this));
    }
    return true;
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

bool PlayerComponent::is_on_ground() const {
    return coyote_timer_ < coyote_time_ || physics_component_->hasCollidedBelow();
}

}  // namespace pyc::sunny_land