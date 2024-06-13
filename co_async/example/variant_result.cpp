#include <chrono>
#include <coroutine>
#include <optional>
#include <variant>

#include "logger/logger.h"

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
        result_.emplace<0>(std::current_exception());
    }

    auto yield_value(int value) {
        result_.emplace<1>(value);
        return std::suspend_always();
    }

    void return_value(int value) { result_.emplace<1>(value); }

    int& result() {
        int* value = std::get_if<1>(&result_);
        if (!value) [[unlikely]] {
            std::rethrow_exception(std::get<0>(result_));
        }
        return *value;
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::coroutine_handle<> previous_ = nullptr;
    std::variant<std::exception_ptr, int> result_;
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