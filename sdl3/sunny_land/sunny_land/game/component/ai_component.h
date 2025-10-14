#pragma once

#include "sunny_land/engine/component/component.h"
#include "sunny_land/engine/utils/macro.h"

namespace pyc::sunny_land {

class AIBehavior;

class TransformComponent;
class SpriteComponent;
class PhysicsComponent;
class AnimationComponent;
class AudioComponent;

/**
 * @brief 处理玩家输入、状态和控制 GameObject 移动的组件。
 *        使用状态模式管理 Idle, Walk, Jump, Fall 等状态。
 */
class AIComponent final : public Component {
    friend class GameObject;

    DECLARE_COMPONENT(TransformComponent, transform_component_);
    DECLARE_COMPONENT(SpriteComponent, sprite_component_);
    DECLARE_COMPONENT(PhysicsComponent, physics_component_);
    DECLARE_COMPONENT(AnimationComponent, animation_component_);
    DECLARE_COMPONENT(AudioComponent, audio_component_);

public:
    void setBehavior(std::unique_ptr<AIBehavior> behavior);  ///< @brief 设置当前 AI 行为策略
    bool takeDamage(int damage);                             ///< @brief 处理伤害逻辑，返回是否造成伤害
    bool isAlive() const;                                    ///< @brief 检查对象是否存活

private:
    // 核心循环函数
    void init() override;
    void update(std::chrono::duration<float> delta_time, Context& context) override;

private:
    std::unique_ptr<AIBehavior> current_behavior_{};
};

}  // namespace pyc::sunny_land