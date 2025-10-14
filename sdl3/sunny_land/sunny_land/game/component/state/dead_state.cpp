#include "sunny_land/game/component/state/dead_state.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/collider_component.h"
#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/game/component/player_component.h"
#include "sunny_land/game/component/state/state_factory.h"

namespace pyc::sunny_land {

void DeadState::enter() {
    spdlog::debug("玩家进入死亡状态。");
    playAnimation("hurt");  // 播放死亡(受伤)动画

    // 应用击退力（只向上）
    auto physics_component = player_component_->getPhysicsComponent();
    physics_component->setVelocity(glm::vec2(0.0f, -200.0f));  // 向上击退

    // 禁用碰撞(自动掉出屏幕)
    if (auto collider_component = player_component_->getOwner()->getComponent<ColliderComponent>()) {
        collider_component->setActive(false);
    }

    playSound("dead");  // 播放死亡音效
}

std::unique_ptr<PlayerState> DeadState::handleInput(Context&) {
    // 死亡状态下不处理输入
    return nullptr;
}

std::unique_ptr<PlayerState> DeadState::update(std::chrono::duration<float>, Context&) {
    // 死亡状态下不更新状态
    return nullptr;
}

}  // namespace pyc::sunny_land