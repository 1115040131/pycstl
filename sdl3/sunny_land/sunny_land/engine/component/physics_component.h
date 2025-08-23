#pragma once

#include <glm/glm.hpp>

#include "sunny_land/engine/component/component.h"

namespace pyc::sunny_land {

class PhysicsEngine;
class TransformComponent;

class PhysicsComponent final : public Component {
    friend class GameObject;
    friend class PhysicsEngine;

public:
    /**
     * @brief 构造函数
     *
     * @param physics_engine 指向PhysicsEngine的指针，不能为nullptr
     * @param use_gravity 物体是否受重力影响，默认true
     * @param mass 物体质量，默认1.0
     */
    PhysicsComponent(PhysicsEngine* physics_engine, bool use_gravity = true, float mass = 1.0f);

    ///< @brief 添加力
    void addForce(const glm::vec2& force) {
        if (enabled_) {
            force_ += force;
        }
    }
    void clearForce() { force_ = {0.0f, 0.0f}; }          ///< @brief 清除力
    const glm::vec2& getForce() const { return force_; }  ///< @brief 获取当前力
    float getMass() const { return mass_; }               ///< @brief 获取质量
    bool isEnabled() const { return enabled_; }           ///< @brief 获取组件是否启用
    bool isUseGravity() const { return use_gravity_; }    ///< @brief 获取组件是否受重力影响

    // 设置器/获取器
    void setEnabled(bool enabled) { enabled_ = enabled; }                      ///< @brief 设置组件是否启用
    void setMass(float mass) { mass_ = std::max(0.0f, mass); }                 ///< @brief 设置质量，质量不能为负
    void setUseGravity(bool use_gravity) { use_gravity_ = use_gravity; }       ///< @brief 设置组件是否受重力影响
    void setVelocity(glm::vec2 velocity) { velocity_ = std::move(velocity); }  ///< @brief 设置速度
    const glm::vec2& getVelocity() const { return velocity_; }                 ///< @brief 获取当前速度
    ///< @brief 获取TransformComponent指针
    TransformComponent* getTransform() const { return transform_; }

private:
    // 核心循环方法
    void init() override;
    void update(std::chrono::duration<float>, Context&) override {}
    void clean() override;

private:
    PhysicsEngine* physics_engine_{};  ///< @brief 指向PhysicsEngine的指针
    TransformComponent* transform_{};  ///< @brief TransformComponent的缓存指针

    glm::vec2 velocity_{};     ///< @brief 物体的速度
    glm::vec2 force_ = {};     ///< @brief 当前帧受到的力
    float mass_ = 1.0f;        ///< @brief 物体质量（默认1.0）
    bool use_gravity_ = true;  ///< @brief 物体是否受重力影响
    bool enabled_ = true;      ///< @brief 组件是否激活
};

}  // namespace pyc::sunny_land