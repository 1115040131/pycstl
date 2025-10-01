#include "sunny_land/engine/component/animation_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/render/animation.h"

namespace pyc::sunny_land {

void AnimationComponent::init() {
    if (!owner_) {
        spdlog::error("AnimationComponent 没有所有者 GameObject!");
        return;
    }
    sprite_component_ = owner_->getComponent<SpriteComponent>();
    if (!sprite_component_) {
        spdlog::error("GameObject '{}' 的 AnimationComponent 需要 SpriteComponent, 但未找到。", owner_->getName());
        return;
    }
}

void AnimationComponent::update(std::chrono::duration<float> delta_time, Context&) {
    // 如果没有正在播放的动画，或者没有当前动画，或者没有精灵组件，或者当前动画没有帧，则直接返回
    if (!is_playing_ || !current_animation_ || !sprite_component_ || current_animation_->isEmpty()) {
        spdlog::trace("{} {} AnimationComponent 更新时没有正在播放的动画或精灵组件为空。", owner_->getName(),
                      owner_->getTag());
        return;
    }

    // 推进计时器
    animation_timer_ += delta_time;

    // 获取当前帧
    const auto& current_frame = current_animation_->getFrame(animation_timer_);

    // 更新精灵组件的源矩形 (使用 SpriteComponent 的新方法)
    sprite_component_->setSourceRect(current_frame.source_rect);

    // 检查非循环动画是否已结束
    if (!current_animation_->isLooping() && animation_timer_ >= current_animation_->getTotalDuration()) {
        is_playing_ = false;
        animation_timer_ = current_animation_->getTotalDuration();  // 停在最后一帧
        if (is_one_shot_removal_) {
            owner_->setNeedRemove(true);
        }
    }
}

void AnimationComponent::addAnimation(std::unique_ptr<Animation> animation) {
    if (!animation) {
        return;
    }
    auto name = std::string(animation->getName());
    animations_[name] = std::move(animation);
}

void AnimationComponent::playAnimation(std::string_view name) {
    auto it = animations_.find(name);
    if (it == animations_.end() || !it->second) {
        spdlog::warn("未找到 GameObject '{}' 的动画 '{}'", name, owner_ ? owner_->getName() : "未知");
        return;
    }

    // 如果已经在播放相同的动画，不重新开始（注释这一段则重新开始播放）
    if (current_animation_ == it->second.get() && is_playing_) {
        return;
    }

    current_animation_ = it->second.get();
    animation_timer_ = std::chrono::duration<float>(0.0f);
    is_playing_ = true;

    // 立即将精灵更新到第一帧
    if (sprite_component_ && !current_animation_->isEmpty()) {
        sprite_component_->setSourceRect(current_animation_->getFrames().front().source_rect);
        spdlog::debug("GameObject '{}' 播放动画 '{}'", owner_ ? owner_->getName() : "未知", name);
    }
}

bool AnimationComponent::isAnimationFinished() const {
    // 如果没有当前动画(说明从未调用过playAnimation)，或者当前动画是循环的，则返回 false
    if (!current_animation_ || current_animation_->isLooping()) {
        return false;
    }
    return animation_timer_ >= current_animation_->getTotalDuration();
}

}  // namespace pyc::sunny_land