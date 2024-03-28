#pragma once

#include <atomic>
#include <functional>
#include <thread>

#include "common/singleton.h"
#include "concurrency/thread_safe_queue/thread_safe_queue.h"

namespace pyc {
namespace concurrency {

/// @brief while 轮询线程池
class SimpleThreadPool : public Singleton<SimpleThreadPool> {
    friend class Singleton<SimpleThreadPool>;

public:
    using Task = std::function<void()>;

    template <typename F>
    void Commit(F&& f) {
        if (!stop_) {
            tasks_.Push(std::forward<F>(f));
        }
    }

    void Start() { stop_ = false; }

    void Stop() { stop_ = true; }

private:
    SimpleThreadPool() {
        auto thread_num = std::thread::hardware_concurrency();
        try {
            for (unsigned int i = 0; i < thread_num; ++i) {
                pool_.emplace_back([this] { WorkerThread(); });
            }
        } catch (const std::exception& e) {
            stop_ = true;
            throw e;
        }
    }

    ~SimpleThreadPool() {
        stop_ = true;
        for (auto& thread : pool_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void WorkerThread() {
        while (!stop_) {
            auto task = tasks_.TryPop();
            if (task) {
                task.value()();
            } else {
                std::this_thread::yield();
            }
        }
    }

private:
    std::atomic_bool stop_{false};
    ThreadSafeQueue<Task> tasks_;
    std::vector<std::jthread> pool_;
};

}  // namespace concurrency
}  // namespace pyc
