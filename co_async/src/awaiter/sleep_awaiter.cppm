module;

#include <chrono>
#include <coroutine>

export module co_async.awaiter.sleep_awaiter;

import co_async.timer_loop;

export namespace pyc::co_async {

struct SleepAwaiter {
    bool await_ready() const { return std::chrono::system_clock::now() >= expire_time_; }

    void await_suspend(std::coroutine_handle<> coroutine) const {
        TimerLoop::GetInstance().addTimer(expire_time_, coroutine);
    }

    void await_resume() const noexcept {}

    std::chrono::system_clock::time_point expire_time_;
};

}  // namespace pyc::co_async
