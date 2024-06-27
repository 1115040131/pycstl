#include <chrono>
#include <coroutine>
#include <queue>
#include <span>
#include <thread>

#include <fmt/chrono.h>

#include "logger/logger.h"

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

    auto yield_value(T value) {
        std::construct_at(&result_, std::move(value));
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
    Uninitialized<T> result_{};
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

    std::queue<std::coroutine_handle<>> ready_queue_{};
    std::priority_queue<TimerEntry, std::vector<TimerEntry>, std::greater<TimerEntry>> timer_table_{};
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
    std::coroutine_handle<> previous_;
};

struct WhenAllAwaiter {
    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const {
        counter_.previous_ = coroutine;
        for (const auto& task : tasks_.subspan(1)) {
            task.coroutine_.resume();
        }
        return tasks_.front().coroutine_;
    }

    constexpr void await_resume() const noexcept {}

    WhenAllCounterBlock& counter_;
    std::span<const ReturnPreviousTask> tasks_;
};

template <typename T>
ReturnPreviousTask whenAllHelper(const auto& task, WhenAllCounterBlock& counter, Uninitialized<T>& result) {
    result.putValue(co_await task);
    --counter.count_;
    if (counter.count_ == 0) {
        co_return counter.previous_;
    }
    co_return nullptr;
}

template <std::size_t... Is, typename... Ts>
Task<std::tuple<typename AwaitableTraits<Ts>::NonVoidRetType...>> whenAllImpl(std::index_sequence<Is...>,
                                                                              Ts&&... ts) {
    WhenAllCounterBlock counter{sizeof...(Ts), nullptr};
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    ReturnPreviousTask tasks[] = {whenAllHelper(std::forward<Ts>(ts), counter, std::get<Is>(result))...};
    co_await WhenAllAwaiter(counter, tasks);
    co_return std::tuple<typename AwaitableTraits<Ts>::NonVoidRetType...>(std::get<Is>(result).moveValue()...);
}

template <Awaitable... Ts>
    requires(sizeof...(Ts) > 0)
auto when_all(Ts&&... ts) {
    return whenAllImpl(std::index_sequence_for<Ts...>{}, std::forward<Ts>(ts)...);
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
    auto [i, j, k] = co_await when_all(hello1(), hello2(), hello2());
    logger.info("1和2等完了");

    co_return i + j + k;
}

}  // namespace pyc

int main() {
    auto task = pyc::hello();
    pyc::getLoop().addTask(task.coroutine_);
    pyc::getLoop().runAll();

    pyc::logger.info("得到 hello 的结果: {}", task.coroutine_.promise().result());
    pyc::logger.info("执行完成");

    return 0;
}