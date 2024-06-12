#include <chrono>
#include <coroutine>
#include <queue>
#include <thread>

#include <fmt/chrono.h>

#include "logger/logger.h"

using namespace std::chrono_literals;

namespace pyc {

inline pyc::Logger logger("Coroutine");

template <typename T>
union Uninitialized {
    T value_;

    Uninitialized() noexcept {}
    Uninitialized(Uninitialized&&) = delete;
    ~Uninitialized() noexcept {}

    T moveValue() {
        T value = std::move(value_);
        std::destroy_at(&value_);
        return value;
    }

    template <typename... Args>
    void putValue(Args&&... args) {
        std::construct_at(&value_, std::forward<Args>(args)...);
    }
};

struct PreviousAwaiter {
    std::coroutine_handle<> previous_ = nullptr;

    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const noexcept {
        if (previous_) {
            return previous_;
        }
        return std::noop_coroutine();
    }

    constexpr void await_resume() const noexcept {}
};

template <typename T>
struct Promise {
    Promise() noexcept {};
    Promise(Promise&&) = delete;
    ~Promise() noexcept {};

    auto initial_suspend() { return std::suspend_always(); }

    auto final_suspend() noexcept { return PreviousAwaiter(previous_); }

    void unhandled_exception() noexcept {
        logger.error("unhandled_exception");
        exception_ = std::current_exception();
    }

    auto yield_value(T value) {
        std::construct_at(&result_, std::move(value));
        return std::suspend_always();
    }

    void return_value(const T& value) { result_.putValue(value); }
    void return_value(T&& value) { result_.putValue(std::move(value)); }

    T result() {
        if (exception_) [[unlikely]] {
            std::rethrow_exception(exception_);
        }
        return result_.moveValue();
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::coroutine_handle<> previous_{};
    std::exception_ptr exception_{};
    Uninitialized<T> result_;
};

template <>
struct Promise<void> {
    Promise() noexcept = default;
    Promise(Promise&&) = delete;
    ~Promise() noexcept = default;

    auto initial_suspend() { return std::suspend_always(); }

    auto final_suspend() noexcept { return PreviousAwaiter(previous_); }

    void unhandled_exception() noexcept {
        logger.error("unhandled_exception");
        result_ = std::current_exception();
    }

    void return_void() { result_ = nullptr; }

    void result() {
        if (result_) [[unlikely]] {
            std::rethrow_exception(result_);
        }
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::coroutine_handle<> previous_ = nullptr;
    std::exception_ptr result_ = nullptr;
};

template <typename T>
struct Task {
    using promise_type = Promise<T>;

    Task(std::coroutine_handle<promise_type> coroutine) noexcept : coroutine_(coroutine) {}

    Task(Task&&) = delete;

    ~Task() { coroutine_.destroy(); }

    struct Awaiter {
        constexpr bool await_ready() const noexcept { return false; }

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            coroutine_.promise().previous_ = coroutine;
            return coroutine_;
        }

        auto await_resume() const { return coroutine_.promise().result(); }

        std::coroutine_handle<promise_type> coroutine_;
    };

    auto operator co_await() const noexcept { return Awaiter(coroutine_); }

    operator std::coroutine_handle<>() const noexcept { return coroutine_; }

    std::coroutine_handle<promise_type> coroutine_;
};

struct Loop {
    Loop& operator=(Loop&&) = delete;

    void addTask(std::coroutine_handle<> task) { ready_queue_.push(task); }

    void addTimer(std::chrono::system_clock::time_point expire_time, std::coroutine_handle<> task) {
        timer_table_.emplace(expire_time, task);
    }

    void runAll() {
        while (!ready_queue_.empty() || !timer_table_.empty()) {
            auto now_time = std::chrono::system_clock::now();
            while (!timer_table_.empty()) {
                const auto& timer = timer_table_.top();
                if (now_time >= timer.expire_time_) {
                    ready_queue_.push(timer.couroutine_);
                    timer_table_.pop();
                } else {
                    break;
                }
            }

            while (!ready_queue_.empty()) {
                auto ready_task = ready_queue_.front();
                ready_queue_.pop();
                ready_task.resume();
            }

            if (!timer_table_.empty() && timer_table_.top().expire_time_ > now_time) {
                logger.debug("No task Loop waiting for {:%S}s",
                             timer_table_.top().expire_time_ - std::chrono::system_clock::now());
                std::this_thread::sleep_until(timer_table_.top().expire_time_);
            }
        }
    }

    struct TimerEntry {
        std::chrono::system_clock::time_point expire_time_;
        std::coroutine_handle<> couroutine_;

        bool operator>(const TimerEntry& that) const noexcept { return expire_time_ > that.expire_time_; }
    };

    std::queue<std::coroutine_handle<>> ready_queue_;
    std::priority_queue<TimerEntry, std::vector<TimerEntry>, std::greater<TimerEntry>> timer_table_;
};

Loop& getLoop() {
    static Loop loop;
    return loop;
}

struct SleepAwaiter {
    bool await_ready() const { return std::chrono::system_clock::now() >= expire_time_; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const {
        getLoop().addTimer(expire_time_, coroutine);
        return std::noop_coroutine();
    }

    void await_resume() const noexcept {}

    std::chrono::system_clock::time_point expire_time_;
};

Task<void> sleep_until(std::chrono::system_clock::time_point expire_time) {
    co_await SleepAwaiter(expire_time);
    co_return;
}

Task<void> sleep_for(std::chrono::system_clock::duration duration) {
    co_await SleepAwaiter(std::chrono::system_clock::now() + duration);
    co_return;
}

struct ReturnPreviousPromise {
    auto initial_suspend() noexcept { return std::suspend_always(); }

    auto final_suspend() noexcept { return PreviousAwaiter(previous_); }

    void unhandled_exception() { throw; }

    void return_value(std::coroutine_handle<> coroutine) noexcept { previous_ = coroutine; }

    auto get_return_object() { return std::coroutine_handle<ReturnPreviousPromise>::from_promise(*this); }

    std::coroutine_handle<> previous_{};

    ReturnPreviousPromise& operator=(ReturnPreviousPromise&&) = delete;
};

struct ReturnPreviousTask {
    using promise_type = ReturnPreviousPromise;

    ReturnPreviousTask(std::coroutine_handle<promise_type> coroutine) noexcept : coroutine_(coroutine) {}

    ReturnPreviousTask(ReturnPreviousTask&&) = delete;

    ~ReturnPreviousTask() { coroutine_.destroy(); }

    std::coroutine_handle<promise_type> coroutine_;
};

struct WhenAllAwaiter {
    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const {
        counter_.previous_ = coroutine;
        getLoop().addTask(task2_.coroutine_);
        return task1_.coroutine_;
    }

    constexpr void await_resume() const noexcept {}

    struct CounterBlock {
        std::size_t count_;
        std::coroutine_handle<> previous_;
    };
    CounterBlock& counter_;
    const ReturnPreviousTask& task1_;
    const ReturnPreviousTask& task2_;
};

template <typename T>
ReturnPreviousTask whenAllHelper(const Task<T>& task, WhenAllAwaiter::CounterBlock& counter,
                                 Uninitialized<T>& result) {
    result.putValue(co_await task);
    --counter.count_;
    if (counter.count_ == 0) {
        co_return counter.previous_;
    }
    co_return nullptr;
}

template <typename T1, typename T2>
Task<std::tuple<T1, T2>> when_all(const Task<T1>& task1, const Task<T2>& task2) {
    WhenAllAwaiter::CounterBlock counter;
    std::tuple<Uninitialized<T1>, Uninitialized<T2>> result;
    counter.count_ = 2;
    co_await WhenAllAwaiter(counter, whenAllHelper(task1, counter, std::get<0>(result)),
                            whenAllHelper(task2, counter, std::get<1>(result)));
    co_return std::tuple<T1, T2>(std::get<0>(result).moveValue(), std::get<1>(result).moveValue());
}

Task<int> hello1() {
    logger.debug("开始睡 1s");
    co_await sleep_for(1s);
    logger.debug("结束睡觉");

    co_return 1;
}

Task<int> hello2() {
    logger.debug("开始睡 2s");
    co_await sleep_for(2s);
    logger.debug("结束睡觉");

    co_return 2;
}

Task<int> hello() {
    logger.info("开始等1和2");
    auto [i, j] = co_await when_all(hello1(), hello2());
    logger.info("1和2等完了");

    co_return i + j;
}

}  // namespace pyc

int main() {
    auto task = pyc::hello();
    pyc::getLoop().addTask(task.coroutine_);
    pyc::getLoop().runAll();

    pyc::logger.info("得到 hello 的结果: {}", task.coroutine_.promise().result());
    pyc::logger.info("执行完成");

    return 0;
}