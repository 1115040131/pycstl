#pragma once

#include <coroutine>

#include "co_async/timer_loop.h"

namespace pyc {
namespace co_async {

struct SleepAwaiter {
    bool await_ready() const { return std::chrono::system_clock::now() >= expire_time_; }

    void await_suspend(std::coroutine_handle<> coroutine) const {
        TimerLoop::GetInstance().addTimer(expire_time_, coroutine);
    }

    void await_resume() const noexcept {}

    std::chrono::system_clock::time_point expire_time_;
};

}  // namespace co_async
}  // namespace pyc
