#include <atomic>

#include <gtest/gtest.h>

#include "concurrency/thread_safe_queue.h"
#include "concurrency/thread_safe_queue_ht.h"
#include "concurrency/thread_safe_stack.h"
#include "test/utils.h"

namespace pyc {
namespace concurrency {

template <typename T>
void PushWhilePop(T& thread_safe_container, const std::size_t kDataNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0))
        << fmt::format("{} 要能被 {} 均分", kDataNum, kThreadNum);

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

template <typename T>
void TryPopWhilePush(T& thread_safe_container, const std::size_t kDataNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0))
        << fmt::format("{} 要能被 {} 均分", kDataNum, kThreadNum);

    bool check[kDataNum] = {false};
    std::atomic<int> fail_time{0};  // pop 失败次数

    auto push = [&](std::size_t data) { thread_safe_container.Emplace(data); };
    auto pop = [&](std::size_t) {
        auto result = thread_safe_container.TryPop();
        if (result.has_value()) {
            check[result.value().data] = true;
        } else {
            ++fail_time;
        }
        return true;
    };
    PushWhilePop(kDataNum, 2 * kDataNum, kThreadNum, push, pop);

    for (std::size_t i = 0; i < kDataNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }
    EXPECT_EQ(fail_time, kDataNum);
    EXPECT_TRUE(thread_safe_container.Empty());
}

TEST(ThreadSafeAdaptorTest, ThreadSafeStackTest) {
    ThreadSafeStack<MyClass> stack;
    PushWhilePop(stack, 10000, 16);
    TryPopWhilePush(stack, 10000, 16);
}

TEST(ThreadSafeAdaptorTest, ThreadSafeQueueTest) {
    ThreadSafeQueue<MyClass> queue;
    PushWhilePop(queue, 10000, 16);
    TryPopWhilePush(queue, 10000, 16);
}

TEST(ThreadSafeAdaptorTest, ThreadSafeQueueHtTest) {
    ThreadSafeQueueHt<MyClass> queue;
    PushWhilePop(queue, 10000, 16);
    TryPopWhilePush(queue, 10000, 16);
}

}  // namespace concurrency
}  // namespace pyc