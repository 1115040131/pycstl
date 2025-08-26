#include "sunny_land/engine/physics/physics_engine.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/collider_component.h"
#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/physics/collision.h"

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
    // 处理对象间碰撞
    checkObjectCollisions();
}

void PhysicsEngine::checkObjectCollisions() {
    collision_pairs_.clear();

    for (size_t i = 0; i + 1 < components_.size(); ++i) {
        auto physics_a = components_[i];
        if (!physics_a || !physics_a->isEnabled()) {
            continue;
        }
        auto object_a = physics_a->getOwner();
        if (!object_a) {
            continue;
        }
        auto collider_a = object_a->getComponent<ColliderComponent>();
        if (!collider_a || !collider_a->isActive()) {
            continue;
        }

        for (size_t j = i + 1; j < components_.size(); ++j) {
            auto physics_b = components_[j];
            if (!physics_b || !physics_b->isEnabled()) {
                continue;
            }
            auto object_b = physics_b->getOwner();
            if (!object_b || object_a == object_b) {
                continue;
            }
            auto collider_b = object_b->getComponent<ColliderComponent>();
            if (!collider_b || !collider_b->isActive()) {
                continue;
            }

            if (checkCollision(*collider_a, *collider_b)) {
                collision_pairs_.emplace_back(object_a, object_b);
            }
        }
    }
}

}  // namespace pyc::sunny_land