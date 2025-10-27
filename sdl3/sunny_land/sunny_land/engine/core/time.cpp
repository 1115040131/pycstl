#include "sunny_land/engine/core/time.h"

#include <SDL3/SDL_timer.h>
#include <spdlog/spdlog.h>

namespace pyc::sunny_land {

using namespace std::chrono_literals;

Time::Time() {
    last_time_ = std::chrono::nanoseconds(SDL_GetTicksNS());
    start_time_ = last_time_;
    spdlog::trace("Time 初始化。Last time: {}", last_time_.count());
}

void Time::update() {
    start_time_ = std::chrono::nanoseconds(SDL_GetTicksNS());
    auto current_delta_time = start_time_ - last_time_;
    if (target_frame_time_ != std::chrono::nanoseconds::zero()) {
        limitFrameRate(current_delta_time);
    }

    auto end_time = std::chrono::nanoseconds(SDL_GetTicksNS());
    delta_time_ = std::chrono::duration<float>(end_time - last_time_);
    last_time_ = end_time;
}

void Time::limitFrameRate(std::chrono::nanoseconds current_delta_time) {
    if (current_delta_time < target_frame_time_) {
        SDL_DelayNS((target_frame_time_ - current_delta_time).count());
    }
}

std::chrono::duration<float> Time::getDeltaTime() const { return delta_time_ * time_scale_; }

std::chrono::duration<float> Time::getUnscaledDeltaTime() const { return delta_time_; }

void Time::setTimeScale(double scale) {
    if (scale < 0.0) {
        spdlog::warn("Time scale 不能为负。Clamping to 0.");
        scale = 0.0;
    }
    time_scale_ = scale;
}

double Time::getTimeScale() const { return time_scale_; }

void Time::setTargetFps(uint64_t fps) {
    target_fps_ = fps;
    if (target_fps_ == 0) {
        target_frame_time_ = std::chrono::nanoseconds::zero();
    } else {
        target_frame_time_ = std::chrono::nanoseconds(1s) / fps;
    }

    spdlog::info("Target FPS 设置为: {} (Frame time: {:.6f}s)", target_fps_,
                 std::chrono::duration<float>(target_frame_time_).count());
}

uint64_t Time::getTargetFps() const { return target_fps_; }

}  // namespace pyc::sunny_land
