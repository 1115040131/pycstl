#pragma once

#include <coroutine>
#include <exception>

#include "co_async/awaiter/previous_awaiter.h"
#include "co_async/utils/uninitialized.h"

#ifdef CO_ASYNC_DEBUG
#include "co_async/utils/logger.h"
#endif

namespace pyc {
namespace co_async {

template <typename T>
struct Promise {
    Promise() noexcept {};
    Promise(Promise&&) = delete;
    ~Promise() noexcept {};

    auto initial_suspend() { return std::suspend_always(); }

    auto final_suspend() noexcept { return PreviousAwaiter(previous_); }

    void unhandled_exception() noexcept {
#ifdef CO_ASYNC_DEBUG
        logger.error("unhandled_exception");
#endif
        exception_ = std::current_exception();
    }

    auto yield_value(const T& value) {
        result_.putValue(value);
        return std::suspend_always();
    }

    auto yield_value(T&& value) {
        result_.putValue(std::move(value));
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
        // logger.error("unhandled_exception");
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

}  // namespace co_async
}  // namespace pyc
