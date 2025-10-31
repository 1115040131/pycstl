#include <atomic>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "concurrency/test/utils.h"
#include "concurrency/thread_safe_queue/simple_thread_safe_queue.h"
#include "concurrency/thread_safe_queue/thread_safe_queue.h"
#include "concurrency/thread_safe_stack/thread_safe_stack.h"

namespace pyc {
namespace concurrency {

template <std::size_t kDataNum, std::size_t kThreadNum, typename T>
void PushWhilePop(T& thread_safe_container) {
    static_assert(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0), "kDataNum 要能被 kThreadNum 均分");

    bool check[kDataNum] = {false};

    auto push = [&](std::size_t data) { thread_safe_container.Emplace(data); };
    auto pop = [&](std::size_t) {
        auto result = thread_safe_container.WaitAndPop();
        check[result.data] = true;
        return true;
    };
    PushWhilePop(kDataNum, kDataNum, kThreadNum, push, pop);

    for (std::size_t i = 0; i < kDataNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }
    EXPECT_TRUE(thread_safe_container.Empty());
}

template <std::size_t kDataNum, std::size_t kThreadNum, typename T>
void TryPopWhilePush(T& thread_safe_container) {
    static_assert(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0), "kDataNum 要能被 kThreadNum 均分");

    bool check[kDataNum] = {false};

    auto push = [&](std::size_t data) { thread_safe_container.Emplace(data); };
    auto pop = [&](std::size_t) {
        auto result = thread_safe_container.TryPop();
        if (result.has_value()) {
            check[result.value().data] = true;
        }
        return result;
    };
    PushWhilePop(kDataNum, kDataNum, kThreadNum, push, pop);

    for (std::size_t i = 0; i < kDataNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }
    EXPECT_TRUE(thread_safe_container.Empty());
}

TEST(ThreadSafeAdaptorTest, ThreadSafeStackTest) {
    ThreadSafeStack<MyClass> stack;
    PushWhilePop<10000, 16>(stack);
    TryPopWhilePush<10000, 16>(stack);
}

TEST(ThreadSafeAdaptorTest, SimpleThreadSafeQueueTest) {
    SimpleThreadSafeQueue<MyClass> queue;
    PushWhilePop<10000, 16>(queue);
    TryPopWhilePush<10000, 16>(queue);
}

TEST(ThreadSafeAdaptorTest, ThreadSafeQueueTest) {
    ThreadSafeQueue<MyClass> queue;
    PushWhilePop<10000, 16>(queue);
    TryPopWhilePush<10000, 16>(queue);
}

}  // namespace concurrency
}  // namespace pyc