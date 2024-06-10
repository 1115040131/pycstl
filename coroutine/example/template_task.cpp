#include <logger/logger.h>

#include <chrono>
#include <coroutine>
#include <optional>

inline pyc::Logger logger("Coroutine");

namespace pyc {

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

    std::coroutine_handle<promise_type> coroutine_;
};

Task<void> void_task() {
    logger.debug("[void_task]");
    co_return;
}

Task<std::string> string_task() {
    logger.debug("[string_task]");
    co_return "aaa";
}

Task<double> double_task() {
    logger.debug("[double_task]");
    co_return 3.14;
}

Task<int> hello() {
    co_await void_task();
    logger.info("[hello] 得到 void_task 返回");

    auto ret = co_await string_task();
    logger.info("[hello] 得到 string_task 的结果: {}", ret);

    auto i = static_cast<int>(co_await double_task());
    logger.info("[hello] 得到 double_task 的结果: {}", i);

    co_return i + 1;
}

}  // namespace pyc

int main() {
    auto task = pyc::hello();
    while (!task.coroutine_.done()) {
        task.coroutine_.resume();
        logger.info("[main] 得到 hello 结果: {}", task.coroutine_.promise().result());
    }

    logger.info("[main] 执行完成");

    return 0;
}