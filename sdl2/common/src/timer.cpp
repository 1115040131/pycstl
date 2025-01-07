#include "sdl2/common/timer.h"

namespace pyc {
namespace sdl2 {

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

}  // namespace sdl2
}  // namespace pyc
