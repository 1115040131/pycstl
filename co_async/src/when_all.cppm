module;

#include <coroutine>
#include <cstddef>
#include <exception>
#include <span>
#include <tuple>
#include <utility>

export module co_async.when_all;

export import co_async.awaiter.concepts;
export import co_async.awaiter.previous_awaiter;
export import co_async.task;
export import co_async.utils.uninitialized;

export namespace pyc::co_async {

struct WhenAllAwaiter {
    struct ControlBlock {
        std::size_t count_;
        std::coroutine_handle<> previous_{nullptr};
        std::exception_ptr exception_{nullptr};
    };

    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const {
        if (tasks_.empty()) {
            return coroutine;
        }
        control_.previous_ = coroutine;
        for (const auto& task : tasks_.subspan(1)) {
            task.coroutine_.resume();
        }
        return tasks_.front().coroutine_;
    }

    void await_resume() const {
        if (control_.exception_) [[unlikely]] {
            std::rethrow_exception(control_.exception_);
        }
    }

    ControlBlock& control_;
    std::span<const ReturnPreviousTask> tasks_;
};

template <typename T>
ReturnPreviousTask whenAllHelper(const auto& task, WhenAllAwaiter::ControlBlock& control,
                                 Uninitialized<T>& result) {
    try {
        result.putValue(co_await task);
    } catch (...) {
        control.exception_ = std::current_exception();
        co_return control.previous_;
    }
    --control.count_;
    if (control.count_ == 0) {
        co_return control.previous_;
    }
    co_return nullptr;
}

template <std::size_t... Is, typename... Ts>
Task<std::tuple<typename AwaitableTraits<Ts>::NonVoidRetType...>> whenAllImpl(std::index_sequence<Is...>,
                                                                              Ts&&... ts) {
    WhenAllAwaiter::ControlBlock control{sizeof...(Ts)};
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    ReturnPreviousTask tasks[] = {whenAllHelper(std::forward<Ts>(ts), control, std::get<Is>(result))...};
    co_await WhenAllAwaiter(control, tasks);
    co_return std::tuple<typename AwaitableTraits<Ts>::NonVoidRetType...>(std::get<Is>(result).moveValue()...);
}

template <Awaitable... Ts>
    requires(sizeof...(Ts) > 0)
auto when_all(Ts&&... ts) {
    return whenAllImpl(std::index_sequence_for<Ts...>{}, std::forward<Ts>(ts)...);
}

}  // namespace pyc::co_async
