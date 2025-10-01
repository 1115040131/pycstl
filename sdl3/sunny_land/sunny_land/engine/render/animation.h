#pragma once

#include <chrono>

#include <SDL3/SDL_rect.h>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

/**
 * @brief 代表动画中的单个帧。
 *
 * 包含纹理图集上的源矩形和该帧的显示持续时间。
 */
struct AnimationFrame {
    SDL_FRect source_rect;                  ///< @brief 纹理图集上此帧的区域
    std::chrono::duration<float> duration;  ///< @brief 此帧显示的持续时间（秒）
};

/**
 * @brief 管理一系列动画帧。
 *
 * 存储动画的帧、总时长、名称和循环行为。
 */
class Animation final : public Noncopyable {
public:
    /**
     * @brief 构造函数
     * @param name 动画的名称。
     * @param loop 动画是否应该循环播放。
     */
    Animation(std::string_view name = "default", bool loop = true);

    /**
     * @brief 向动画添加一帧。
     *
     * @param source_rect 纹理图集上此帧的区域。
     * @param duration 此帧应显示的持续时间（秒）。
     */
    void addFrame(SDL_FRect source_rect, std::chrono::duration<float> duration);

    /**
     * @brief 获取在给定时间点应该显示的动画帧。
     * @param time 当前时间（秒）。如果动画循环，则可以超过总持续时间。
     * @return 对应时间点的动画帧。
     */
    const AnimationFrame& getFrame(std::chrono::duration<float> time) const;

    // --- Setters and Getters ---
    std::string_view getName() const { return name_; }                        ///< @brief 获取动画名称。
    const std::vector<AnimationFrame>& getFrames() const { return frames_; }  ///< @brief 获取动画帧列表。
    size_t getFrameCount() const { return frames_.size(); }                   ///< @brief 获取帧数量。
    ///< @brief 获取动画的总持续时间（秒）。
    std::chrono::duration<float> getTotalDuration() const { return total_duration_; }
    bool isLooping() const { return loop_; }          ///< @brief 检查动画是否循环播放。
    bool isEmpty() const { return frames_.empty(); }  ///< @brief 检查动画是否没有帧。

    void setName(std::string_view name) { name_ = name; }  ///< @brief 设置动画名称。
    void setLooping(bool loop) { loop_ = loop; }           ///< @brief 设置动画是否循环播放。

private:
    std::string name_;                             ///< @brief 动画的名称 (例如, "walk", "idle")。
    std::vector<AnimationFrame> frames_;           ///< @brief 动画帧列表
    std::chrono::duration<float> total_duration_;  ///< @brief 动画的总持续时间（秒）
    bool loop_ = true;                             ///< @brief 默认动画是循环的
};

}  // namespace pyc::sunny_land