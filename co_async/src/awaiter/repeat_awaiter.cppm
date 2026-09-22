module;

#include <coroutine>

export module co_async.awaiter.repeat_awaiter;

export namespace pyc::co_async {

struct RepeatAwaiter {
    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        if (coroutine.done()) {
            return std::noop_coroutine();
        }
        return coroutine;
    }

    constexpr void await_resume() const noexcept {}
};

}  // namespace pyc::co_async
