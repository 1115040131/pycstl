#pragma once

#include <atomic>
#include <future>
#include <thread>

#include "common/singleton.h"
#include "concurrency/thread_safe_queue/thread_safe_queue.h"

namespace pyc {
namespace concurrency {

/// @brief 任务窃取线程池
class StealThreadPool : public Singleton<StealThreadPool> {
    friend class Singleton<StealThreadPool>;

public:
    using Task = std::packaged_task<void()>;

    template <typename F, typename... Args>
    auto Commit(F&& f, Args&&... args) {
        using RetType = decltype(std::forward<F>(f)(std::forward<Args>(args)...));
        if (stop_) {
            return std::future<RetType>{};
        }

        std::size_t index = (thread_index.load() + 1) % task_queues_.size();
        thread_index.store(index);
        std::packaged_task<RetType()> task(
            [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable { return f(args...); });
        std::future<RetType> result = task.get_future();
        task_queues_[index].Push(std::move(task));
        return result;
    }

    void Start() { stop_ = false; }

    void Stop() { stop_ = true; }

private:
    StealThreadPool() {
        auto thread_num = std::thread::hardware_concurrency();
        try {
            task_queues_ = std::vector<ThreadSafeQueue<Task>>(thread_num);
            for (unsigned int i = 0; i < thread_num; ++i) {
                pool_.emplace_back([this, i] { WorkerThread(i); });
            }
        } catch (const std::exception& e) {
            stop_ = true;
            task_queues_.clear();
            throw e;
        }
    }

    ~StealThreadPool() {
        stop_ = true;
        for (auto& thread : pool_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void WorkerThread(std::size_t index) {
        while (!stop_) {
            auto pop_task = task_queues_[index].TryPop();
            if (pop_task) {
                pop_task.value()();
                continue;
            }

            bool steal_result = false;
            for (std::size_t i = 0; i < task_queues_.size(); i++) {
                if (i == index) {
                    continue;
                }

                auto steal_task = task_queues_[i].TrySteal();
                if (steal_task) {
                    steal_task.value()();
                    steal_result = true;
                    break;
                }
            }

            if (steal_result) {
                continue;
            }

            std::this_thread::yield();
        }
    }

private:
    std::atomic_bool stop_{false};
    std::atomic_int thread_index{0};
    std::vector<ThreadSafeQueue<Task>> task_queues_;
    std::vector<std::jthread> pool_;
};

}  // namespace concurrency
}  // namespace pyc
