#include "sunny_land/game/component/ai/updown_behavior.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/animation_component.h"
#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/game/component/ai_component.h"

namespace pyc::sunny_land {

UpDownBehavior::UpDownBehavior(float min_y, float max_y, float speed)
    : patrol_min_y_(min_y), patrol_max_y_(max_y), move_speed_(speed) {
    if (patrol_min_y_ >= patrol_max_y_) {
        spdlog::error("UpDownBehavior: min_x ({}) 应小于 max_x ({})。行为可能不正确。", min_y, max_y);
        patrol_min_y_ = patrol_max_y_;  // 修正为相等，避免逻辑错误
    }
}

void UpDownBehavior::enter(AIComponent& ai_component) {
    if (auto animation_component = ai_component.getAnimationComponent()) {
        animation_component->playAnimation("fly");
    }

    // 禁用重力
    if (auto physics_component = ai_component.getPhysicsComponent()) {
        physics_component->setUseGravity(false);
    }
}

void UpDownBehavior::update(std::chrono::duration<float>, AIComponent& ai_component) {
    // 获取必要的组件
    auto physics_component = ai_component.getPhysicsComponent();
    auto transform_component = ai_component.getTransformComponent();
    auto sprite_component = ai_component.getSpriteComponent();
    if (!physics_component || !transform_component || !sprite_component) {
        spdlog::error("UpDownBehavior: 缺少必要的组件, 无法执行巡逻行为。");
        return;
    }

    // --- 检查碰撞和边界 ---
    auto current_y = transform_component->getPosition().y;

    // 到达上边界或碰到上方障碍，向下移动
    if (physics_component->hasCollidedAbove() || current_y <= patrol_min_y_) {
        physics_component->setVelocityY(move_speed_);
        moving_down_ = true;
        // 到达下边界或碰到下方障碍，向上移动
    } else if (physics_component->hasCollidedBelow() || current_y >= patrol_max_y_) {
        physics_component->setVelocityY(-move_speed_);
        moving_down_ = false;
    }

    /* 不需要翻转精灵图 */
}

}  // namespace pyc::sunny_land