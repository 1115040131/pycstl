#include <chrono>
#include <coroutine>
#include <map>
#include <span>
#include <thread>
#include <unordered_map>
#include <variant>

#include <fmt/chrono.h>

#include "logger/logger.h"

// 检查GCC版本是否小于 13
#if defined(__GNUC__) && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ < 130000)
namespace std {
template <>
struct hash<std::coroutine_handle<>> {
    size_t operator()(const std::coroutine_handle<>& handle) const noexcept {
        return hash<void*>{}(handle.address());
    }
};
}  // namespace std
#endif

using namespace std::chrono_literals;

namespace pyc {

inline pyc::Logger logger("Coroutine");

template <typename T = void>
struct NonVoidHelper {
    using Type = T;
};

template <>
struct NonVoidHelper<void> {
    using Type = NonVoidHelper;

    explicit NonVoidHelper() noexcept = default;
};

template <typename T>
struct Uninitialized {
    union {
        T value_;
    };

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

template <>
struct Uninitialized<void> {
    auto moveValue() { return NonVoidHelper<>(); }

    void putValue(NonVoidHelper<>) {}
};

template <typename T>
struct Uninitialized<const T> : Uninitialized<T> {};

template <typename T>
struct Uninitialized<T&> : Uninitialized<std::reference_wrapper<T>> {};

template <typename T>
struct Uninitialized<T&&> : Uninitialized<T> {};

template <typename A>
concept Awaiter = requires(A a, std::coroutine_handle<> h) {
    { a.await_ready() } -> std::same_as<bool>;
    { a.await_suspend(h) };
    { a.await_resume() };
};

template <typename A>
concept Awaitable = Awaiter<A> || requires(A a) {
    { a.operator co_await() } -> Awaiter;
};

template <typename A>
struct AwaitableTraits {};

template <Awaiter A>
struct AwaitableTraits<A> {
    using RetType = decltype(std::declval<A>().await_resume());
    using NonVoidRetType = NonVoidHelper<RetType>::Type;
};

template <typename A>
    requires(!Awaiter<A> && Awaitable<A>)
struct AwaitableTraits<A> : AwaitableTraits<decltype(std::declval<A>().operator co_await())> {};

struct Loop {
    Loop& operator=(Loop&&) = delete;

    void addTimer(std::chrono::system_clock::time_point expire_time, std::coroutine_handle<> task) {
        auto [iter, _] = timer_map_.emplace(expire_time, task);
        search_table_.emplace(task, iter);
    }

    void deleteTask(std::coroutine_handle<> task) {
        auto search = search_table_.find(task);
        if (search != search_table_.end()) {
            timer_map_.erase(search->second);
            search_table_.erase(search);
        }
    }

    void runAll() {
        while (!timer_map_.empty()) {
            auto now_time = std::chrono::system_clock::now();
            auto [expire_time, task] = *timer_map_.begin();
            if (now_time >= expire_time) {
                deleteTask(task);
                task.resume();
            } else {
                logger.debug("No task Loop waiting for {:%S}s", expire_time - now_time);
                std::this_thread::sleep_until(expire_time);
            }
        }
    }

    using TimerMap = std::map<std::chrono::system_clock::time_point, std::coroutine_handle<>>;
    TimerMap timer_map_;
    std::unordered_map<std::coroutine_handle<>, TimerMap::iterator> search_table_;
};

Loop& getLoop() {
    static Loop loop;
    return loop;
}

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

struct SleepAwaiter {
    bool await_ready() const { return std::chrono::system_clock::now() >= expire_time_; }

    void await_suspend(std::coroutine_handle<> coroutine) const { getLoop().addTimer(expire_time_, coroutine); }

    void await_resume() const noexcept {}

    std::chrono::system_clock::time_point expire_time_;
};

template <typename T>
struct TimerTask : public Task<T> {
    using Task<T>::Task;

    ~TimerTask() { getLoop().deleteTask(this->coroutine_); }
};

TimerTask<void> sleep_until(std::chrono::system_clock::time_point expire_time) {
    co_await SleepAwaiter(expire_time);
    co_return;
}

TimerTask<void> sleep_for(std::chrono::system_clock::duration duration) {
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

struct WhenAllCounterBlock {
    std::size_t count_;
    std::coroutine_handle<> previous_{nullptr};
    std::exception_ptr exception_{nullptr};
};

struct WhenAllAwaiter {
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

    WhenAllCounterBlock& control_;
    std::span<const ReturnPreviousTask> tasks_;
};

template <typename T>
ReturnPreviousTask whenAllHelper(const auto& task, WhenAllCounterBlock& control, Uninitialized<T>& result) {
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
    WhenAllCounterBlock control{sizeof...(Ts)};
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
    auto var = co_await when_any(hello2(), hello1(), hello2());
    logger.info("看到 {} 睡醒了", var.index() + 1);

    if (var.index() == 0) {
        co_return std::get<0>(var);
    } else if (var.index() == 1) {
        co_return std::get<1>(var);
    } else {
        co_return std::get<2>(var);
    }
}

}  // namespace pyc

int main() {
    auto task = pyc::hello();
    task.coroutine_.resume();
    pyc::getLoop().runAll();

    pyc::logger.info("得到 hello 的结果: {}", task.coroutine_.promise().result());
    pyc::logger.info("执行完成");

    return 0;
}