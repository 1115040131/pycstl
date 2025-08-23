#include "sunny_land/engine/component/physics_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/physics/physics_engine.h"

namespace pyc::sunny_land {

PhysicsComponent::PhysicsComponent(PhysicsEngine* physics_engine, bool use_gravity, float mass)
    : physics_engine_(physics_engine), mass_(std::max(0.0f, mass)), use_gravity_(use_gravity) {
    if (!physics_engine_) {
        spdlog::error("PhysicsComponent构造函数中, PhysicsEngine指针不能为nullptr!");
    }
    spdlog::trace("物理组件创建完成，质量: {}, 使用重力: {}", mass_, use_gravity_);
}

void PhysicsComponent::init() {
    if (!owner_) {
        spdlog::error("物理组件初始化前需要一个GameObject作为所有者!");
        return;
    }
    if (!physics_engine_) {
        spdlog::error("物理组件初始化时, PhysicsEngine未正确初始化。");
        return;
    }
    transform_ = owner_->getComponent<TransformComponent>();
    if (!transform_) {
        spdlog::warn("物理组件初始化时, 同一GameObject上没有找到TransformComponent组件。");
    }
    // 注册到PhysicsEngine
    physics_engine_->registerComponent(this);
    spdlog::trace("物理组件初始化完成。");
}

void PhysicsComponent::clean() {
    physics_engine_->unregisterComponent(this);
    spdlog::trace("物理组件清理完成。");
}

}  // namespace pyc::sunny_land