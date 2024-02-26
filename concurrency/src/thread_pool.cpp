#include "concurrency/thread_pool.h"

namespace pyc {
namespace concurrency {

void ThreadPool::Start() {
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

void ThreadPool::Stop() {
    stop_ = true;
    cv_lock_.notify_all();
    for (auto& thread : pool_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

}  // namespace concurrency
}  // namespace pyc
