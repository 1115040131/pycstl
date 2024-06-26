#pragma once

#include <chrono>

#include "co_async/awaiter/sleep_awaiter.h"
#include "co_async/task.h"

namespace pyc {
namespace co_async {

template <typename T>
struct TimerTask : public Task<T> {
    using Task<T>::Task;

    ~TimerTask() { TimerLoop::GetInstance().deleteTask(this->coroutine_); }
};

inline TimerTask<void> sleep_until(std::chrono::system_clock::time_point expire_time) {
    co_await SleepAwaiter(expire_time);
    co_return;
}

inline TimerTask<void> sleep_for(std::chrono::system_clock::duration duration) {
    co_await SleepAwaiter(std::chrono::system_clock::now() + duration);
    co_return;
}

}  // namespace co_async
}  // namespace pyc
