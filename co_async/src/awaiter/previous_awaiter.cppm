module;

#include <coroutine>

export module co_async.awaiter.previous_awaiter;

export namespace pyc::co_async {

struct PreviousAwaiter {
    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const noexcept {
        if (previous_) {
            return previous_;
        }
        return std::noop_coroutine();
    }

    constexpr void await_resume() const noexcept {}

    std::coroutine_handle<> previous_ = nullptr;
};

}  // namespace pyc::co_async
