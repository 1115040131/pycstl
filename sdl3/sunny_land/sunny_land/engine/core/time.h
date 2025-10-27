#pragma once

#include <chrono>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

/**
 * @brief 管理游戏循环中的时间，计算帧间时间差
 *
 */
class Time final : Noncopyable {
public:
    Time();

    /**
     * @brief 每帧开始调用，更新内部时间状态并计算 DeltaTime
     */
    void update();

    /**
     * @brief 获取经过时间缩放调整后的 DeltaTime
     *
     * @return std::chrono::duration<float> 缩放后的 DeltaTime (s)
     */
    std::chrono::duration<float> getDeltaTime() const;

    /**
     * @brief 获取未经过时间缩放调整后的 DeltaTime
     *
     * @return std::chrono::duration<float> 未缩放后 DeltaTime (s)
     */
    std::chrono::duration<float> getUnscaledDeltaTime() const;

    /**
     * @brief 设置时间缩放因子。
     *
     * @param scale 时间缩放值。1.0 为正常速度，< 1.0 为慢动作，> 1.0 为快进。
     *              不允许负值。
     */
    void setTimeScale(double scale);

    /**
     * @brief 获取当前的时间缩放因子。
     *
     * @return double 当前的时间缩放因子。
     */
    double getTimeScale() const;

    /**
     * @brief 设置目标帧率。
     *
     * @param fps 目标每秒帧数。设置为 0 表示不限制帧率
     */
    void setTargetFps(uint64_t fps);

    /**
     * @brief 获取当前设置的目标帧率。
     *
     * @return uint64_t 目标 FPS，0 表示不限制。
     */
    uint64_t getTargetFps() const;

private:
    /**
     * @brief update 中调用，用于限制帧率。如果设置了 target_fps_ > 0 且当前帧执行时间小于目标帧，则调用
     *        SDL_DelayNS
     *
     * @param current_delta_time 当前帧执行时间 (s)
     */
    void limitFrameRate(std::chrono::nanoseconds current_delta_time);

private:
    std::chrono::nanoseconds last_time_{};       // 上一帧的时间戳
    std::chrono::nanoseconds start_time_{};      // 当前帧开始时间戳
    std::chrono::duration<float> delta_time_{};  // 未缩放的帧间时间差
    double time_scale_{1.0};                     // 时间缩放因子

    uint64_t target_fps_{};
    std::chrono::nanoseconds target_frame_time_{};
};

}  // namespace pyc::sunny_land