module;

#include <chrono>
#include <memory>

export module sunny_land.game.ai:ai_component;

import sunny_land.engine.component.animation_component;
import sunny_land.engine.component.audio_component;
export import sunny_land.engine.core;

export namespace pyc::sunny_land {

class AIBehavior;

/**
 * @brief 处理玩家输入、状态和控制 GameObject 移动的组件。
 *        使用状态模式管理 Idle, Walk, Jump, Fall 等状态。
 */
class AIComponent final : public Component {
    friend class GameObject;

public:
    TransformComponent* getTransformComponent() const { return transform_component_; }
    SpriteComponent* getSpriteComponent() const { return sprite_component_; }
    PhysicsComponent* getPhysicsComponent() const { return physics_component_; }
    AnimationComponent* getAnimationComponent() const { return animation_component_; }
    AudioComponent* getAudioComponent() const { return audio_component_; }

    ~AIComponent();

    void setBehavior(std::unique_ptr<AIBehavior> behavior);  ///< @brief 设置当前 AI 行为策略
    bool takeDamage(int damage);                             ///< @brief 处理伤害逻辑，返回是否造成伤害
    bool isAlive() const;                                    ///< @brief 检查对象是否存活

private:
    // 核心循环函数
    void init() override;
    void update(std::chrono::duration<float> delta_time, Context& context) override;

private:
    TransformComponent* transform_component_{nullptr};
    SpriteComponent* sprite_component_{nullptr};
    PhysicsComponent* physics_component_{nullptr};
    AnimationComponent* animation_component_{nullptr};
    AudioComponent* audio_component_{nullptr};

    std::unique_ptr<AIBehavior> current_behavior_{};
};

}  // namespace pyc::sunny_land
