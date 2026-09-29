module;

#include <chrono>

module sdl2.common.timer;

namespace pyc::sdl2 {

void Timer::on_update(std::chrono::duration<double> delta) {
    if (paused_) {
        return;
    }

    pass_time_ += delta;
    if (pass_time_ >= wait_time_) {
        bool can_shot = !one_shot_ || !shotted_;
        if (can_shot && on_timeout_) {
            on_timeout_();
            shotted_ = true;
        }
        pass_time_ -= wait_time_;
    }
}

}  // namespace pyc::sdl2
