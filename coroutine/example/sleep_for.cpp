#include <logger/logger.h>

#include <chrono>
#include <coroutine>
#include <queue>
#include <thread>

#include <fmt/chrono.h>

using namespace std::chrono_literals;

namespace pyc {

inline pyc::Logger logger("Coroutine");

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

    void return_value(T value) { std::construct_at(&result_, std::move(value)); }

    T result() {
        if (exception_) [[unlikely]] {
            std::rethrow_exception(exception_);
        }
        return std::move(result_);
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::coroutine_handle<> previous_{};
    std::exception_ptr exception_{};
    // union 成员不会被初始化
    union {
        T result_;
    };
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

    auto operator co_await() { return Awaiter(coroutine_); }

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

Task<int> hello1() {
    logger.info("[hello1] 开始睡 1s");
    co_await sleep_for(1s);
    logger.info("[hello1] 结束睡觉");

    co_return 1;
}

Task<int> hello2() {
    logger.info("[hello2] 开始睡 2s");
    co_await sleep_for(2s);
    logger.info("[hello2] 结束睡觉");

    co_return 2;
}

}  // namespace pyc

int main() {
    auto task1 = pyc::hello1();
    auto task2 = pyc::hello2();
    pyc::getLoop().addTask(task1);
    pyc::getLoop().addTask(task2);
    pyc::getLoop().runAll();

    pyc::logger.info("[main] 得到 hello1 的结果: {}", task1.coroutine_.promise().result());
    pyc::logger.info("[main] 得到 hello2 的结果: {}", task2.coroutine_.promise().result());
    pyc::logger.info("[main] 执行完成");

    return 0;
}