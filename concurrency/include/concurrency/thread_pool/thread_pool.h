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
    friend class Singleton<ThreadPool>;

public:
    using Task = std::packaged_task<void()>;

    template <typename F, typename... Args>
    auto Commit(F&& f, Args&&... args) {
        using RetType = decltype(std::forward<F>(f)(std::forward<Args>(args)...));
        if (stop_) {
            return std::future<RetType>{};
        }
        std::packaged_task<RetType()> task(
            [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable {
                return f(std::forward<Args>(args)...);
            });

        std::future<RetType> result = task.get_future();
        {
            std::lock_guard<std::mutex> lock(mtx_);
            tasks_.emplace([task = std::move(task)]() mutable { task(); });
        }
        cv_lock_.notify_one();
        return result;
    }

    int IdleThreadCount() const { return thread_num_; }

private:
    ThreadPool(unsigned int num = 5) {
        thread_num_ = std::max(num, 1u);
        Start();
    }

    ~ThreadPool() { Stop(); }

    void Start() {
        for (int i = 0; i < thread_num_; ++i) {
            pool_.emplace_back([this]() {
                while (!stop_) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(mtx_);
                        cv_lock_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                        if (stop_) {
                            return;
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    thread_num_--;
                    task();
                    thread_num_++;
                }
            });
        }
    }

    void Stop() {
        stop_ = true;
        cv_lock_.notify_all();
        for (auto& thread : pool_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

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
