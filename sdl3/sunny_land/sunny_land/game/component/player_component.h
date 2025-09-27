#pragma once

#include <glm/glm.hpp>

#include "sunny_land/engine/component/component.h"
#include "sunny_land/game/component/state/player_state.h"

namespace pyc::sunny_land {

class TransformComponent;
class SpriteComponent;
class PhysicsComponent;

/**
 * @brief 处理玩家输入、状态和控制 GameObject 移动的组件。
 *        使用状态模式管理 Idle, Walk, Jump, Fall 等状态。
 */
class PlayerComponent final : public Component {
    friend class GameObject;

public:
    // setters and getters
    TransformComponent* getTransformComponent() const { return transform_component_; }
    SpriteComponent* getSpriteComponent() const { return sprite_component_; }
    PhysicsComponent* getPhysicsComponent() const { return physics_component_; }

    void setIsDead(bool is_dead) { is_dead_ = is_dead; }               ///< @brief 设置玩家是否死亡
    bool isDead() const { return is_dead_; }                           ///< @brief 获取玩家是否死亡
    void setMoveForce(float move_force) { move_force_ = move_force; }  ///< @brief 设置水平移动力
    float getMoveForce() const { return move_force_; }                 ///< @brief 获取水平移动力
    void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }      ///< @brief 设置最大移动速度
    float getMaxSpeed() const { return max_speed_; }                   ///< @brief 获取最大移动速度
    void setFrictionFactor(float friction_factor) { friction_factor_ = friction_factor; }  ///< @brief 设置摩擦系数
    float getFrictionFactor() const { return friction_factor_; }                           ///< @brief 获取摩擦系数
    void setJumpVelocity(float jump_vel) { jump_vel_ = jump_vel; }                         ///< @brief 设置跳跃速度
    float getJumpVelocity() const { return jump_vel_; }                                    ///< @brief 获取跳跃速度

    void setState(std::unique_ptr<PlayerState> new_state);  ///< @brief 切换玩家状态

private:
    // 核心循环函数
    void init() override;
    void handleInput(Context& context) override;
    void update(std::chrono::duration<float> delta_time, Context& context) override;

private:
    TransformComponent* transform_component_ = nullptr;  // 指向 TransformComponent 的非拥有指针
    SpriteComponent* sprite_component_ = nullptr;
    PhysicsComponent* physics_component_ = nullptr;

    std::unique_ptr<PlayerState> current_state_;
    bool is_dead_{false};

    // --- 移动相关参数
    float move_force_ = 200.0f;      ///< @brief 水平移动力
    float max_speed_ = 120.0f;       ///< @brief 最大移动速度 (像素/秒)
    float friction_factor_ = 0.85f;  ///< @brief 摩擦系数 (Idle时缓冲效果，每帧乘以此系数)
    float jump_vel_ = 350.0f;        ///< @brief 跳跃速度 (按下"jump"键给的瞬间向上的速度)
};

}  // namespace pyc::sunny_land