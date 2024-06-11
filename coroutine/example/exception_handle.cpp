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

struct Promise {
    auto initial_suspend() { return std::suspend_always(); }

    auto final_suspend() noexcept { return PreviousAwaiter(previous_); }

    void unhandled_exception() noexcept {
        logger.error("unhandled_exception");
        exception_ptr = std::current_exception();
    }

    auto yield_value(int value) {
        value_ = value;
        return std::suspend_always();
    }

    void return_value(int value) { value_ = value; }

    int& result() {
        if (exception_ptr) [[unlikely]] {
            std::rethrow_exception(exception_ptr);
        }
        return value_.value();
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::coroutine_handle<> previous_ = nullptr;
    std::exception_ptr exception_ptr{};
    std::optional<int> value_{};
};

struct Task {
    using promise_type = Promise;

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

Task world() {
    throw std::runtime_error("world 失败");
    co_return 41;
}

Task hello() {
    int i = co_await world();

    co_return i + 1;
}

}  // namespace pyc

int main() {
    auto task = pyc::hello();
    while (!task.coroutine_.done()) {
        task.coroutine_.resume();
        logger.info("get hello value: {}", task.coroutine_.promise().result());
    }

    logger.info("执行完成");

    return 0;
}