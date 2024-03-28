#pragma once

#include <atomic>
#include <condition_variable>
#include <exception>
#include <future>
#include <mutex>
#include <thread>

namespace pyc {
namespace concurrency {

class ThreadInterrupted : public std::exception {
public:
    const char* what() const noexcept override { return "Thread interrupted"; }
};

void InterruptionPoint();

class InterruptFlag {
public:
    InterruptFlag() = default;

    void Set() {
        flag_.store(true, std::memory_order_relaxed);
        std::lock_guard<std::mutex> lock(set_clear_mutex_);
        if (thread_cond_) {
            thread_cond_->notify_all();
        } else if (thread_cond_any_) {
            thread_cond_any_->notify_all();
        }
    }

    bool IsSet() const { return flag_.load(std::memory_order_relaxed); }

    void SetConditionVariable(std::condition_variable& cv) {
        std::lock_guard<std::mutex> lock(set_clear_mutex_);
        thread_cond_ = &cv;
    }

    void ClearConditionVariable() {
        std::lock_guard<std::mutex> lock(set_clear_mutex_);
        thread_cond_ = nullptr;
    }

    template <typename Lock>
    void Wait(std::condition_variable_any& cv, Lock& lk) {
        struct CustomLock {
            InterruptFlag* self;
            Lock& lk;

            CustomLock(InterruptFlag* self_, std::condition_variable_any& cv, Lock& lk_) : self(self_), lk(lk_) {
                self->set_clear_mutex_.lock();
                self->thread_cond_any_ = &cv;
            }

            ~CustomLock() {
                self->thread_cond_any_ = nullptr;
                self->set_clear_mutex_.unlock();
            }

            void lock() { std::lock(self->set_clear_mutex_, lk); }

            void unlock() {
                lk.unlock();
                self->set_clear_mutex_.unlock();
            }
        };

        CustomLock custom_lock(this, cv, lk);
        InterruptionPoint();
        cv.wait(custom_lock);
        InterruptionPoint();
    }

private:
    std::atomic<bool> flag_{false};
    std::condition_variable* thread_cond_{nullptr};
    std::condition_variable_any* thread_cond_any_{nullptr};
    std::mutex set_clear_mutex_;
};

extern thread_local InterruptFlag this_thread_interrupt_flag;

class InterruptibleThread {
public:
    template <typename Function, typename... Args>
    explicit InterruptibleThread(Function&& f, Args&&... args) {
        std::promise<InterruptFlag*> p;
        thread_ = std::jthread([f = std::forward<Function>(f), ... args = std::forward<Args>(args), &p]() mutable {
            p.set_value(&this_thread_interrupt_flag);
            f(std::forward<Args>(args)...);
        });
        flag_ = p.get_future().get();
    }

    void Join() { thread_.join(); }

    void Interrupt() {
        if (flag_) {
            flag_->Set();
        }
    }

private:
    std::jthread thread_;
    InterruptFlag* flag_;
};

struct ClearInterruptFlag {
    ~ClearInterruptFlag() { this_thread_interrupt_flag.ClearConditionVariable(); }
};

void InterruptibleWait(std::condition_variable& cv, std::unique_lock<std::mutex>& lock);

template <typename Predicate>
void InterruptibleWait(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, Predicate pred) {
    InterruptionPoint();
    this_thread_interrupt_flag.SetConditionVariable(cv);
    ClearInterruptFlag guard;
    while (!pred()) {
        cv.wait_for(lock, std::chrono::milliseconds(1));
    }
    InterruptionPoint();
}

template <typename T>
void InterruptibleWait(std::future<T>& future) {
    while (!this_thread_interrupt_flag.IsSet()) {
        if (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
            break;
        }
    }
    InterruptionPoint();
}

}  // namespace concurrency
}  // namespace pyc
