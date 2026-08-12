#include "co_async/timer_loop.h"

#include <thread>

#include "co_async/utils/debug.h"

namespace pyc {
namespace co_async {

void TimerLoop::addTimer(std::chrono::system_clock::time_point expire_time, std::coroutine_handle<> task) {
    auto [iter, _] = timer_map_.emplace(expire_time, task);
    search_table_.emplace(task, iter);
}

void TimerLoop::deleteTask(std::coroutine_handle<> task) {
    auto search = search_table_.find(task);
    if (search != search_table_.end()) {
        timer_map_.erase(search->second);
        search_table_.erase(search);
    }
}

void TimerLoop::runAll() {
    while (!timer_map_.empty()) {
        auto now_time = std::chrono::system_clock::now();
        auto [expire_time, task] = *timer_map_.begin();
        if (now_time >= expire_time) {
            deleteTask(task);
            task.resume();
        } else {
            CO_ASYNC_LOG_DEBUG("No task Loop waiting for {:%S}s", expire_time - now_time);
            std::this_thread::sleep_until(expire_time);
        }
    }
}

}  // namespace co_async
}  // namespace pyc
