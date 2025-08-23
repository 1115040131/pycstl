#pragma once

#include <chrono>
#include <vector>

#include <glm/glm.hpp>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

class PhysicsComponent;

class PhysicsEngine : Noncopyable {
public:
    void registerComponent(PhysicsComponent* component);    ///< @brief 注册物理组件
    void unregisterComponent(PhysicsComponent* component);  ///< @brief 注销物理组件

    void update(std::chrono::duration<float> delta_time);  ///< @brief 核心循环：更新所有注册的物理组件的状态

    // 设置器/获取器
    void setGravity(glm::vec2 gravity) { gravity_ = std::move(gravity); }  ///< @brief 设置全局重力加速度
    const glm::vec2& getGravity() const { return gravity_; }               ///< @brief 获取当前的全局重力加速度
    void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }          ///< @brief 设置最大速度
    float getMaxSpeed() const { return max_speed_; }                       ///< @brief 获取当前的最大速度

private:
    std::vector<PhysicsComponent*> components_;  ///< @brief 注册的物理组件容器，非拥有指针
    glm::vec2 gravity_ = {0.0f, 980.0f};         ///< @brief 默认重力值 (像素/秒^2, 相当于100像素对应现实1m)
    float max_speed_ = 500.0f;                   ///< @brief 最大速度 (像素/秒)
};

}  // namespace pyc::sunny_land