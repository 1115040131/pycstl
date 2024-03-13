#include <atomic>

#include <gtest/gtest.h>

#include "concurrency/thread_safe_queue.h"
#include "concurrency/thread_safe_queue_ht.h"
#include "concurrency/thread_safe_stack.h"
#include "test/utils.h"

namespace pyc {
namespace concurrency {

template <typename T>
void PopWhilePush(T& thread_safe_container, const std::size_t kMaxNum, const std::size_t kThreadNum) {
    const std::size_t kBlockSize = kMaxNum / kThreadNum;
    bool check[kMaxNum] = {false};

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_FALSE(check[i]);
    }

    std::vector<std::function<void(std::size_t)>> actions;
    // kThreadNum 个线程同时 Push [0, kMaxNum) 的数据
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize; j++) {
            thread_safe_container.Emplace(kStart + j);
        }
    });
    // kThreadNum 个线程同时 Pop [0, kMaxNum) 的数据
    actions.emplace_back([&](std::size_t) {
        for (std::size_t j = 0; j < kBlockSize; j++) {
            auto result = thread_safe_container.WaitAndPop();
            EXPECT_TRUE(result.data < static_cast<int>(kMaxNum));
            check[result.data] = true;
        }
    });
    MultiThreadExecute(kThreadNum, actions);

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }
}

template <typename T>
void TryPopWhilePush(T& thread_safe_container, const std::size_t kMaxNum, const std::size_t kThreadNum) {
    const std::size_t kBlockSize = kMaxNum / kThreadNum;
    bool check[kMaxNum] = {false};
    std::atomic<int> fail_time{0};  // pop 失败次数

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_FALSE(check[i]);
    }

    std::vector<std::function<void(std::size_t)>> actions;
    // kThreadNum 个线程同时 Push [0, kMaxNum) 的数据
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize; j++) {
            thread_safe_container.Emplace(kStart + j);
        }
    });
    // kThreadNum 个线程同时 Pop [0, kMaxNum) 的数据
    actions.emplace_back([&](std::size_t) {
        for (std::size_t j = 0; j < kBlockSize * 2; j++) {
            auto result = thread_safe_container.TryPop();
            if (result.has_value()) {
                EXPECT_TRUE(result.value().data < static_cast<int>(kMaxNum));
                check[result.value().data] = true;
            } else {
                ++fail_time;
            }
        }
    });
    MultiThreadExecute(kThreadNum, actions);

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }
    EXPECT_EQ(fail_time, kMaxNum);
}

TEST(ThreadSafeAdaptorTest, ThreadSafeStackTest) {
    ThreadSafeStack<MyClass> stack;
    PopWhilePush(stack, 10000, 16);
    EXPECT_TRUE(stack.Empty());

    TryPopWhilePush(stack, 10000, 16);
    EXPECT_TRUE(stack.Empty());
}

TEST(ThreadSafeAdaptorTest, ThreadSafeQueueTest) {
    ThreadSafeQueue<MyClass> queue;
    PopWhilePush(queue, 10000, 16);
    EXPECT_TRUE(queue.Empty());

    TryPopWhilePush(queue, 10000, 16);
    EXPECT_TRUE(queue.Empty());
}

TEST(ThreadSafeAdaptorTest, ThreadSafeQueueHtTest) {
    ThreadSafeQueueHt<MyClass> queue;
    PopWhilePush(queue, 10000, 16);
    EXPECT_TRUE(queue.Empty());

    TryPopWhilePush(queue, 10000, 16);
    EXPECT_TRUE(queue.Empty());
}

}  // namespace concurrency
}  // namespace pyc