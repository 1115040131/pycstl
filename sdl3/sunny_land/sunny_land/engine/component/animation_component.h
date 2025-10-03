#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

#include "common/string_hash.h"
#include "sunny_land/engine/component/component.h"

namespace pyc::sunny_land {

class Animation;

class SpriteComponent;

/**
 * @brief GameObject的动画组件。
 *
 * 持有一组Animation对象并控制其播放，
 * 根据当前帧更新关联的SpriteComponent。
 */
class AnimationComponent final : public Component {
    friend class GameObject;

public:
    void addAnimation(std::unique_ptr<Animation> animation);  ///< @brief 向 animations_ map容器中添加一个动画。
    void playAnimation(std::string_view name);                ///< @brief 播放指定名称的动画。
    void stopAnimation() { is_playing_ = false; }             ///< @brief 停止当前动画播放。
    void resumeAnimation() { is_playing_ = true; }            ///< @brief 恢复当前动画播放。
    bool isAnimationFinished() const;

    // --- Getters and Setters ---
    std::string_view getCurrentAnimationName() const;
    bool isPlaying() const { return is_playing_; }
    bool isOneShotRemoval() const { return is_one_shot_removal_; }
    void setOneShotRemoval(bool is_one_shot_removal) { is_one_shot_removal_ = is_one_shot_removal; }

protected:
    // 核心循环方法
    void init() override;
    void update(std::chrono::duration<float>, Context&) override;

private:
    /// @brief 动画名称到Animation对象的映射。
    std::unordered_map<std::string, std::unique_ptr<Animation>, StringHash, StringEqual> animations_;
    SpriteComponent* sprite_component_ = nullptr;  ///< @brief 指向必需的SpriteComponent的指针
    Animation* current_animation_ = nullptr;       ///< @brief 指向当前播放动画的原始指针

    std::chrono::duration<float> animation_timer_{};  ///< @brief 动画播放中的计时器
    bool is_playing_ = false;                         ///< @brief 当前是否有动画正在播放
    bool is_one_shot_removal_ = false;                ///< @brief 是否在动画结束后删除整个GameObject
};

}  // namespace pyc::sunny_land