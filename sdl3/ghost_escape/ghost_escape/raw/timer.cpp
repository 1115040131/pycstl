#include "ghost_escape/raw/timer.h"

namespace pyc {
namespace sdl3 {

Timer* Timer::CreateAndSet(Object* parent, std::chrono::duration<float> interval) {
    auto timer = std::make_unique<Timer>();
    timer->init();
#ifdef DEBUG_MODE
    timer->SET_NAME(Timer);
#endif
    timer->setActive(false);
    timer->interval_ = interval;
    return static_cast<Timer*>(parent->addChild(std::move(timer)));
}

void Timer::update(std::chrono::duration<float> delta) {
    timer_ += delta;
    if (timer_ >= interval_) {
        time_out_ = true;
        timer_ = std::chrono::duration<float>::zero();
    }
}

bool Timer::isTimeOut() {
    if (time_out_) {
        time_out_ = false;
        return true;
    }
    return false;
}

}  // namespace sdl3
}  // namespace pyc