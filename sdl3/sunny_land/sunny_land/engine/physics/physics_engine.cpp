#include "sunny_land/engine/physics/physics_engine.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/transform_component.h"

namespace pyc::sunny_land {

void PhysicsEngine::registerComponent(PhysicsComponent* component) {
    components_.push_back(component);
    spdlog::trace("物理组件注册完成。");
}

void PhysicsEngine::unregisterComponent(PhysicsComponent* component) {
    std::erase(components_, component);
    spdlog::trace("物理组件注销完成。");
}

void PhysicsEngine::update(std::chrono::duration<float> delta_time) {
    for (auto physics : components_) {
        if (!physics || !physics->isEnabled()) {
            continue;
        }

        // 应用重力 (如果组件受重力影响)：F = g * m
        if (physics->isUseGravity()) {
            physics->addForce(gravity_ * physics->getMass());
        }

        // 更新速度： v += a * dt，其中 a = F / m
        physics->velocity_ += (physics->getForce() / physics->getMass()) * delta_time.count();
        physics->clearForce();

        // 更新位置: S += v * dt
        auto transform = physics->getTransform();
        if (transform) {
            transform->translate(physics->velocity_ * delta_time.count());
        }

        // 限制最大速度
        physics->velocity_ = glm::clamp(physics->velocity_, -max_speed_, max_speed_);
    }
}

}  // namespace pyc::sunny_land