#include "monster_war/engine/system/animation_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include "monster_war/engine/component/animation_component.h"
#include "monster_war/engine/component/sprite_component.h"

namespace pyc::monster_war {

AnimationSystem::AnimationSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<PlayAnimationEvent>().connect<&AnimationSystem::onPlayAnimationEvent>(this);
}

AnimationSystem::~AnimationSystem() { dispatcher_.disconnect(this); }

void AnimationSystem::update(std::chrono::duration<float> delta_time) {
    auto view = registry_.view<AnimationComponent, SpriteComponent>();
    for (auto [entity, animation, sprite] : view.each()) {
        // 如果动画不存在, 则跳过
        auto it = animation.animations_.find(animation.current_animation_id_);
        if (it == animation.animations_.end()) {
            continue;
        }

        // 获取当前动画
        auto& current_animation = it->second;
        // 如果没有动画帧, 则跳过
        if (current_animation.frames_.empty()) {
            continue;
        }

        // 更新动画计时器
        animation.current_time_ += animation.speed_ * delta_time;

        // 获取当前帧
        const auto& current_frame = current_animation.frames_[animation.current_frame_index_];

        // 检查是否切换下一帧
        if (animation.current_time_ >= current_frame.duration_) {
            animation.current_time_ -= current_frame.duration_;
            animation.current_frame_index_++;

            // 处理动画播放完成
            if (animation.current_frame_index_ >= current_animation.frames_.size()) {
                if (current_animation.loop_) {
                    animation.current_frame_index_ = 0;
                } else {
                    // 动画播放完毕且不循环，停在最后一帧
                    animation.current_frame_index_ = current_animation.frames_.size() - 1;
                    // 发送动画播放完成事件
                    dispatcher_.enqueue(AnimationFinishedEvent{entity, animation.current_animation_id_});
                }
            }
        }

        // 更新 SpriteComponent 的源矩形 （根据当前动画帧的源矩形信息）
        sprite.sprite_.src_rect_ = current_animation.frames_[animation.current_frame_index_].src_rect_;
    }
}

void AnimationSystem::onPlayAnimationEvent(const PlayAnimationEvent& event) {
    // 使用try_get方法来安全获取可能存在的组件。如果不存在则返回nullptr
    if (auto anim = registry_.try_get<AnimationComponent>(event.entity_)) {
        anim->current_animation_id_ = event.animation_id_;
        anim->current_frame_index_ = 0;
        anim->current_time_ = {};
        anim->animations_.at(event.animation_id_).loop_ = event.loop_;
    }
}

}  // namespace pyc::monster_war