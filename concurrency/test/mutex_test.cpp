
#include <exception>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <stack>
#include <thread>

#include <fmt/core.h>
#include <gtest/gtest.h>

#include "common/noncopyable.h"
#include "common/utils.h"

namespace pyc {
namespace concurrency {

using namespace std::literals::chrono_literals;

#pragma region 线程安全 stack

template <typename T>
class UnsafeStack {
public:
    explicit UnsafeStack() = default;

    UnsafeStack(const UnsafeStack& other) {
        std::lock_guard<std::mutex> lock(other.mutex_);
        data_ = other.data_;
    }

    UnsafeStack& operator=(const UnsafeStack&) = delete;

    virtual ~UnsafeStack() = default;

    void Push(T&& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_.push(std::forward<T>(value));
    }

    // 问题代码
    virtual T Pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto element = data_.top();
        data_.pop();
        return element;
    }

    bool Empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.empty();
    }

protected:
    std::stack<T> data_;
    mutable std::mutex mutex_;
};

TEST(DISABLED_MutexTest, UnsafeStackTest) {
    UnsafeStack<int> unsafe_stack;

    unsafe_stack.Push(1);

    std::jthread t1([&unsafe_stack] {
        if (!unsafe_stack.Empty()) {
            std::this_thread::sleep_for(10ms);
            unsafe_stack.Pop();
        }
    });

    std::jthread t2([&unsafe_stack] {
        if (!unsafe_stack.Empty()) {
            std::this_thread::sleep_for(50ms);
            unsafe_stack.Pop();  // 未定义行为
        }
    });
}

struct empty_stack : std::exception {
    const char* what() const noexcept override { return "empty stack"; }
};

template <typename T>
class SafeStack : public UnsafeStack<T> {
public:
    T Pop() override {
        std::lock_guard<std::mutex> lock(this->mutex_);
        if (this->data_.empty()) {
            throw empty_stack();
        }
        auto element = this->data_.top();
        this->data_.pop();
        return element;
    }

    void Pop(T& value) {
        std::lock_guard<std::mutex> lock(this->mutex_);
        if (this->data_.empty()) {
            return;
        }
        value = this->data_.top();
        this->data_.pop();
    }

    std::optional<T> PopOptional() {
        std::lock_guard<std::mutex> lock(this->mutex_);
        if (this->data_.empty()) {
            return {};
        }
        auto element = this->data_.top();
        this->data_.pop();
        return element;
    }

    std::shared_ptr<T> PopSharedPtr() {
        std::lock_guard<std::mutex> lock(this->mutex_);
        if (this->data_.empty()) {
            return nullptr;
        }
        auto result = std::make_shared<T>(this->data_.top());
        this->data_.pop();
        return result;
    }
};

TEST(MutexTest, SafeStackTest) {
    SafeStack<int> safe_stack;

    safe_stack.Push(1);
    EXPECT_EQ(safe_stack.Pop(), 1);
    EXPECT_ANY_THROW(safe_stack.Pop());

    safe_stack.Push(1);
    {
        int top_element{};
        safe_stack.Pop(top_element);
        EXPECT_EQ(top_element, 1);
    }
    {
        int top_element{};
        safe_stack.Pop(top_element);
        EXPECT_EQ(top_element, int{});
    }

    safe_stack.Push(1);
    {
        EXPECT_EQ(safe_stack.PopOptional().value(), 1);
        EXPECT_FALSE(safe_stack.PopOptional());
    }

    safe_stack.Push(1);
    EXPECT_EQ(*safe_stack.PopSharedPtr(), 1);
    EXPECT_FALSE(safe_stack.PopSharedPtr());
}

#pragma endregion

#pragma region 线程安全 swap

struct BigObject {
    int data;
};

class BigObjectManager {
public:
    explicit BigObjectManager(int data) : obj_{data} {}

    int GetObjectData() const { return obj_.data; }

    friend void DangerSwap(BigObjectManager& lhs, BigObjectManager& rhs);

    friend void SafeSwap(BigObjectManager& lhs, BigObjectManager& rhs);

    friend void SafeSwapScope(BigObjectManager& lhs, BigObjectManager& rhs);

private:
    std::mutex mutex_;
    BigObject obj_;
};

void DangerSwap(BigObjectManager& lhs, BigObjectManager& rhs) {
    fmt::println("thread [{}] begin", GetThreadId());
    if (&lhs == &rhs) {
        return;
    }

    std::lock_guard<std::mutex> guard1(lhs.mutex_);
    // 此处为了让死锁必现, sleep 一段时间
    std::this_thread::sleep_for(10ms);
    std::lock_guard<std::mutex> guard2(rhs.mutex_);

    std::swap(lhs.obj_, rhs.obj_);
    fmt::println("thread [{}] end", GetThreadId());
}

void SafeSwap(BigObjectManager& lhs, BigObjectManager& rhs) {
    fmt::println("thread [{}] begin", GetThreadId());
    if (&lhs == &rhs) {
        return;
    }

    std::lock(lhs.mutex_, rhs.mutex_);
    // 领养锁, 管理锁的释放, 不管理加锁
    std::lock_guard<std::mutex> guard1(lhs.mutex_, std::adopt_lock);
    std::this_thread::sleep_for(10ms);
    std::lock_guard<std::mutex> guard2(rhs.mutex_, std::adopt_lock);

    std::swap(lhs.obj_, rhs.obj_);
    fmt::println("thread [{}] end", GetThreadId());
}

void SafeSwapScope(BigObjectManager& lhs, BigObjectManager& rhs) {
    fmt::println("thread [{}] begin", GetThreadId());
    if (&lhs == &rhs) {
        return;
    }

    std::scoped_lock guard(lhs.mutex_, rhs.mutex_);

    std::swap(lhs.obj_, rhs.obj_);
    fmt::println("thread [{}] end", GetThreadId());
}

// 发生死锁
TEST(DISABLED_MutexTest, DangerSwapTest) {
    BigObjectManager manager1{1};
    BigObjectManager manager2{2};
    std::jthread t1(DangerSwap, std::ref(manager1), std::ref(manager2));
    std::jthread t2(DangerSwap, std::ref(manager2), std::ref(manager1));
}

TEST(MutexTest, SafeSwapTest) {
    constexpr int data1 = 1;
    constexpr int data2 = 10;
    BigObjectManager manager1{data1};
    BigObjectManager manager2{data2};
    {
        std::jthread t1(SafeSwap, std::ref(manager1), std::ref(manager2));
        std::jthread t2(SafeSwap, std::ref(manager2), std::ref(manager1));
    }
    EXPECT_EQ(manager1.GetObjectData(), data1);
    EXPECT_EQ(manager2.GetObjectData(), data2);
    {
        std::jthread t1(SafeSwapScope, std::ref(manager1), std::ref(manager2));
        std::jthread t2(SafeSwapScope, std::ref(manager2), std::ref(manager1));
    }
    EXPECT_EQ(manager1.GetObjectData(), data1);
    EXPECT_EQ(manager2.GetObjectData(), data2);
}

#pragma endregion

#pragma region 层级锁

// 层级锁
class HierarchicalMutex : public Noncopyable {
public:
    explicit HierarchicalMutex(std::size_t value) : current_hierarchy_value_(value) {}

    void Lock() {
        CheckForHierarchyViolation();
        mutex_.lock();
        UpdateHierarchyValue();
    }

    void Unlock() {
        if (thread_hierarchy_value_ != current_hierarchy_value_) {
            throw std::logic_error("mutex hierarchy violated");
        }

        thread_hierarchy_value_ = previous_hierarchy_value_;
        mutex_.unlock();
    }

    bool try_lock() {
        if (!mutex_.try_lock()) {
            return false;
        }
        UpdateHierarchyValue();
        return true;
    }

private:
    void CheckForHierarchyViolation() {
        if (thread_hierarchy_value_ <= current_hierarchy_value_) {
            throw std::logic_error("mutex hierarchy violated");
        }
    }

    void UpdateHierarchyValue() {
        previous_hierarchy_value_ = thread_hierarchy_value_;
        thread_hierarchy_value_ = current_hierarchy_value_;
    }

private:
    std::mutex mutex_;
    std::size_t current_hierarchy_value_;                     // 当前层级值
    std::size_t previous_hierarchy_value_{0};                 // 上一层级值
    static thread_local std::size_t thread_hierarchy_value_;  // 当前线程的层级值
};

thread_local std::size_t HierarchicalMutex::thread_hierarchy_value_{std::numeric_limits<std::size_t>::max()};

TEST(MutextTest, HierarchicalMutexTest) {
    HierarchicalMutex hmtx1(1000);
    HierarchicalMutex hmtx2(500);
    {
        std::jthread t1([&hmtx1, &hmtx2]() {
            hmtx1.Lock();
            std::this_thread::sleep_for(10ms);
            hmtx2.Lock();
            hmtx2.Unlock();
            hmtx1.Unlock();
        });
        std::jthread t2([&hmtx1, &hmtx2]() {
            hmtx2.Lock();
            std::this_thread::sleep_for(10ms);
            EXPECT_THROW(hmtx1.Lock(), std::logic_error);
            EXPECT_THROW(hmtx1.Unlock(), std::logic_error);
            hmtx2.Unlock();
        });
    }
}

#pragma endregion

}  // namespace concurrency
}  // namespace pyc