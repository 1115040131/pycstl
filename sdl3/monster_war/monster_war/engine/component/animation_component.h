#pragma once

#include <chrono>
#include <unordered_map>
#include <vector>

#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>

#include "monster_war/engine/utils/math.h"

namespace pyc::monster_war {

using namespace std::chrono_literals;

/**
 * @brief 动画帧数据结构
 *
 * 包含帧源矩形和帧间隔。
 */
struct AnimationFrame {
    Rect src_rect_{};                               ///< @brief 帧源矩形
    std::chrono::duration<float> duration_{100ms};  ///< @brief 帧间隔
};

/**
 * @brief 动画数据结构
 *
 * 包含动画名称、帧列表、总时长、当前播放时间、是否循环等属性。
 */
struct Animation {
    std::vector<AnimationFrame> frames_;                ///< @brief 动画帧
    std::unordered_map<size_t, entt::id_type> events_;  ///< @brief 动画事件，键为帧索引，值为事件ID
    std::chrono::duration<float> total_duration_{};     ///< @brief 动画总时长
    bool loop_{true};                                   ///< @brief 是否循环

    /**
     * @brief 构造函数
     * @param name 动画名称
     * @param frames 动画帧
     * @param events 动画事件，默认为空
     * @param loop 是否循环，默认true
     */
    explicit Animation(std::vector<AnimationFrame> frames, std::unordered_map<size_t, entt::id_type> events = {},
                       bool loop = true)
        : frames_(std::move(frames)), events_(std::move(events)), loop_(loop) {
        // 计算动画总时长 (总时长 = 所有帧时长之和)
        total_duration_ = {};
        for (const auto& frame : frames_) {
            total_duration_ += frame.duration_;
        }
    }
};

/**
 * @brief 动画组件
 *
 * 包含动画名称、帧列表、总时长、当前播放时间、是否循环等属性。
 */
struct AnimationComponent {
    std::unordered_map<entt::id_type, Animation> animations_;  ///< @brief 动画集合
    entt::id_type current_animation_id_{entt::null};           ///< @brief 当前播放的动画名称
    size_t current_frame_index_{};                             ///< @brief 当前播放的帧索引
    std::chrono::duration<float> current_time_{};              ///< @brief 当前播放时间
    float speed_{1.0f};                                        ///< @brief 播放速度
};

}  // namespace pyc::monster_war