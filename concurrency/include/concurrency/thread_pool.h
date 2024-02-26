#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

#include "common/singleton.h"

namespace pyc {
namespace concurrency {

class ThreadPool : public Singleton<ThreadPool> {
public:
    using Task = std::packaged_task<void()>;

    ThreadPool(unsigned int num = 5) {
        thread_num_ = std::max(num, 1u);
        Start();
    }

    ~ThreadPool() { Stop(); }

    template <typename F, typename... Args>
    auto Commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using RetType = decltype(f(args...));
        if (stop_) {
            return std::future<RetType>{};
        }
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<RetType> result = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mtx_);
            tasks_.emplace([task] { (*task)(); });
        }
        cv_lock_.notify_one();
        return result;
    }

    int IdleThreadCount() const { return thread_num_; }

private:
    void Start();

    void Stop();

private:
    std::mutex mtx_;
    std::condition_variable cv_lock_;
    std::atomic_bool stop_{false};
    std::atomic_int thread_num_;
    std::queue<Task> tasks_;
    std::vector<std::thread> pool_;
};

}  // namespace concurrency
}  // namespace pyc
