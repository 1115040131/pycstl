#include "sunny_land/game/component/ai_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/animation_component.h"
#include "sunny_land/engine/component/health_component.h"
#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/game/component/ai/ai_behavior.h"

namespace pyc::sunny_land {

void AIComponent::init() {
    if (!owner_) {
        spdlog::error("AIComponent 没有所属游戏对象!");
        return;
    }

    // 获取必要的组件
    transform_component_ = owner_->getComponent<TransformComponent>();
    physics_component_ = owner_->getComponent<PhysicsComponent>();
    sprite_component_ = owner_->getComponent<SpriteComponent>();
    animation_component_ = owner_->getComponent<AnimationComponent>();

    // 检查必要组件是否存在
    if (!transform_component_ || !physics_component_ || !sprite_component_ || !animation_component_) {
        spdlog::error("{} AIComponent 对象缺少必要组件！", owner_->getName());
    }
}

void AIComponent::update(std::chrono::duration<float> delta_time, Context&) {
    // 将更新委托给当前的行为策略
    if (current_behavior_) {
        current_behavior_->update(delta_time, *this);
    } else {
        spdlog::warn("GameObject '{}' 上的 AIComponent 没有设置行为。", owner_ ? owner_->getName() : "Unknown");
    }
}

void AIComponent::setBehavior(std::unique_ptr<AIBehavior> behavior) {
    current_behavior_ = std::move(behavior);
    spdlog::debug("GameObject '{}' 上的 AIComponent 设置了新的行为。", owner_ ? owner_->getName() : "Unknown");
    if (current_behavior_) {
        current_behavior_->enter(*this);  // 调用新行为的 enter 方法
    }
}

bool AIComponent::takeDamage(int damage) {
    if (auto health_component = getOwner()->getComponent<HealthComponent>()) {
        return health_component->takeDamage(damage);
    }
    return false;
}

bool AIComponent::isAlive() const {
    if (auto health_component = getOwner()->getComponent<HealthComponent>()) {
        return health_component->isAlive();
    }
    return true;  // 如果没有 HealthComponent，则默认认为是存活的
}

}  // namespace pyc::sunny_land