#pragma once

#include <coroutine>
#include <exception>
#include <span>
#include <variant>

#include "co_async/awaiter/concepts.h"
#include "co_async/task.h"

namespace pyc {
namespace co_async {

struct WhenAnyCounterBlock {
    static constexpr std::size_t kNullIndex = std::size_t(-1);

    std::size_t index_{kNullIndex};
    std::coroutine_handle<> previous_;
    std::exception_ptr exception_;
};

struct WhenAnyAwaiter {
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

    WhenAnyCounterBlock& control_;
    std::span<const ReturnPreviousTask> tasks_;
};

template <typename T>
ReturnPreviousTask whenAnyHelper(const auto& task, WhenAnyCounterBlock& control, Uninitialized<T>& result,
                                 std::size_t index) {
    try {
        result.putValue(co_await task);
    } catch (...) {
        control.exception_ = std::current_exception();
        co_return control.previous_;
    }
    control.index_ = index;
    co_return control.previous_;
}

template <std::size_t... Is, typename... Ts>
Task<std::variant<typename AwaitableTraits<Ts>::NonVoidRetType...>> whenAnyImpl(std::index_sequence<Is...>,
                                                                                Ts&&... ts) {
    WhenAnyCounterBlock control{};
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    ReturnPreviousTask tasks[] = {whenAnyHelper(std::forward<Ts>(ts), control, std::get<Is>(result), Is)...};
    co_await WhenAnyAwaiter(control, tasks);
    Uninitialized<std::variant<typename AwaitableTraits<Ts>::NonVoidRetType...>> var_result;
    ((control.index_ == Is && (var_result.putValue(std::in_place_index<Is>, std::get<Is>(result).moveValue()), 0)),
     ...);
    co_return var_result.moveValue();
}

template <Awaitable... Ts>
    requires(sizeof...(Ts) > 0)
auto when_any(Ts&&... ts) {
    return whenAnyImpl(std::index_sequence_for<Ts...>{}, std::forward<Ts>(ts)...);
}

}  // namespace co_async
}  // namespace pyc
