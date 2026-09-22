module;

#include <chrono>
#include <coroutine>

export module co_async.sleep;

import co_async.awaiter.sleep_awaiter;
export import co_async.task;
import co_async.timer_loop;

export namespace pyc::co_async {

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

}  // namespace pyc::co_async
