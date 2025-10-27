#pragma once

#include <glm/glm.hpp>

#include "sunny_land/engine/component/component.h"
#include "sunny_land/game/component/state/player_state.h"

namespace pyc::sunny_land {

using namespace std::chrono_literals;

class TransformComponent;
class SpriteComponent;
class PhysicsComponent;
class AnimationComponent;
class AudioComponent;
class HealthComponent;

/**
 * @brief 处理玩家输入、状态和控制 GameObject 移动的组件。
 *        使用状态模式管理 Idle, Walk, Jump, Fall 等状态。
 */
class PlayerComponent final : public Component {
    friend class GameObject;

    DECLARE_COMPONENT(TransformComponent, transform_component_);
    DECLARE_COMPONENT(SpriteComponent, sprite_component_);
    DECLARE_COMPONENT(PhysicsComponent, physics_component_);
    DECLARE_COMPONENT(AnimationComponent, animation_component_);
    DECLARE_COMPONENT(AudioComponent, audio_component_);
    DECLARE_COMPONENT(HealthComponent, health_component_);

public:
    bool takeDamage(int damage);  ///< @brief 试图造成伤害，返回是否成功

    // setters and getters
    void setIsDead(bool is_dead) { is_dead_ = is_dead; }                   ///< @brief 设置玩家是否死亡
    bool isDead() const { return is_dead_; }                               ///< @brief 获取玩家是否死亡
    void setMoveForce(float move_force) { move_force_ = move_force; }      ///< @brief 设置水平移动力
    float getMoveForce() const { return move_force_; }                     ///< @brief 获取水平移动力
    void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }          ///< @brief 设置最大移动速度
    float getMaxSpeed() const { return max_speed_; }                       ///< @brief 获取最大移动速度
    void setClimbSpeed(float climb_speed) { climb_speed_ = climb_speed; }  ///< @brief 设置爬梯子速度
    float getClimbSpeed() const { return climb_speed_; }                   ///< @brief 获取爬梯子速度
    void setFrictionFactor(float friction_factor) { friction_factor_ = friction_factor; }  ///< @brief 设置摩擦系数
    float getFrictionFactor() const { return friction_factor_; }                           ///< @brief 获取摩擦系数
    void setJumpVelocity(float jump_vel) { jump_vel_ = jump_vel; }                         ///< @brief 设置跳跃速度
    float getJumpVelocity() const { return jump_vel_; }                                    ///< @brief 获取跳跃速度
    ///< @brief 设置硬直时间
    void setStunnedDuration(std::chrono::duration<float> duration) { stunned_duration_ = duration; }
    std::chrono::duration<float> getStunnedDuration() const { return stunned_duration_; }  ///< @brief 获取硬直时间

    void setState(std::unique_ptr<PlayerState> new_state);  ///< @brief 切换玩家状态
    bool is_on_ground() const;                              ///< @brief 检查玩家是否在地面上(考虑了Coyote Time)

private:
    // 核心循环函数
    void init() override;
    void handleInput(Context& context) override;
    void update(std::chrono::duration<float> delta_time, Context& context) override;

private:
    std::unique_ptr<PlayerState> current_state_;
    bool is_dead_{false};

    // --- 移动相关参数
    float move_force_ = 200.0f;      ///< @brief 水平移动力
    float max_speed_ = 120.0f;       ///< @brief 最大移动速度 (像素/秒)
    float climb_speed_ = 100.0f;     ///< @brief 爬梯子速度 (像素/秒)
    float friction_factor_ = 0.85f;  ///< @brief 摩擦系数 (Idle时缓冲效果，每帧乘以此系数)
    float jump_vel_ = 350.0f;        ///< @brief 跳跃速度 (按下"jump"键给的瞬间向上的速度)

    // --- 属性相关参数 ---
    std::chrono::duration<float> stunned_duration_{0.4f};  ///< @brief 玩家被击中后的硬直时间（单位：秒）

    // 土狼时间(Coyote Time): 允许玩家在离地后短暂时间内仍然可以跳跃
    static constexpr auto coyote_time_ = 0.1s;         ///< @brief Coyote Time (单位：秒）
    std::chrono::duration<float> coyote_timer_{0.0f};  ///< @brief Coyote Time 计时器

    // 无敌闪烁时间
    static constexpr auto flash_interval_ = 0.03s;    ///< @brief 闪烁间隔时间（单位：秒）
    std::chrono::duration<float> flash_timer_{0.0f};  ///< @brief 闪烁计时器，用于无敌状态下的闪烁效果
};

}  // namespace pyc::sunny_land