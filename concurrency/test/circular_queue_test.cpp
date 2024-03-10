#include <iostream>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "concurrency/circular_queue_light.h"
#include "concurrency/circular_queue_lock.h"
#include "concurrency/circular_queue_seq.h"
#include "concurrency/circular_queue_sync.h"

namespace pyc {
namespace concurrency {

struct MyClass {
    int data;
};

template <template <typename, std::size_t, typename> class QueueType>
void TestCircularQueue(std::size_t thread_num) {
    QueueType<MyClass, 100, std::allocator<MyClass>> queue;
    constexpr std::size_t kMaxNum = 20000;
    const std::size_t kBlockSize = kMaxNum / thread_num;
    std::atomic<std::size_t> count{0};
    bool check[kMaxNum] = {false};

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_FALSE(check[i]);
    }

    {
        std::vector<std::jthread> push_threads;
        push_threads.reserve(thread_num);
        std::vector<std::jthread> pop_threads;
        pop_threads.reserve(thread_num);

        // thread_num 个线程同时将数据 Push
        for (std::size_t i = 0; i < thread_num; i++) {
            push_threads.emplace_back([&, i]() {
                const std::size_t kStart = i * kBlockSize;
                for (std::size_t j = 0; j < kBlockSize; j++) {
                    while (!queue.Push(MyClass(kStart + j))) {
                    }
                }
            });
        }

        // thread_num 个线程同时 Pop 数据
        for (std::size_t i = 0; i < thread_num; i++) {
            push_threads.emplace_back([&]() {
                for (;;) {
                    auto result = queue.Pop();
                    if (result.has_value()) {
                        check[result.value().data] = true;
                        ++count;
                    }
                    if (count == kMaxNum) {
                        break;
                    }
                }
            });
        }
    }

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }
}

TEST(CircularQueueTest, CircularQueueLock1Thread) { TestCircularQueue<CircularQueueLock>(1); }
TEST(CircularQueueTest, CircularQueueLock2Thread) { TestCircularQueue<CircularQueueLock>(2); }
TEST(CircularQueueTest, CircularQueueLock4Thread) { TestCircularQueue<CircularQueueLock>(4); }
TEST(CircularQueueTest, CircularQueueLock8Thread) { TestCircularQueue<CircularQueueLock>(8); }
TEST(CircularQueueTest, CircularQueueLock16Thread) { TestCircularQueue<CircularQueueLock>(16); }

TEST(CircularQueueTest, CircularQueueSeq1Thread) { TestCircularQueue<CircularQueueSeq>(1); }
TEST(CircularQueueTest, CircularQueueSeq2Thread) { TestCircularQueue<CircularQueueSeq>(2); }
TEST(CircularQueueTest, CircularQueueSeq4Thread) { TestCircularQueue<CircularQueueSeq>(4); }
TEST(CircularQueueTest, CircularQueueSeq8Thread) { TestCircularQueue<CircularQueueSeq>(8); }
TEST(CircularQueueTest, CircularQueueSeq16Thread) { TestCircularQueue<CircularQueueSeq>(16); }

TEST(CircularQueueTest, CircularQueueLight1Thread) { TestCircularQueue<CircularQueueLight>(1); }
TEST(CircularQueueTest, CircularQueueLight2Thread) { TestCircularQueue<CircularQueueLight>(2); }
TEST(CircularQueueTest, CircularQueueLight4Thread) { TestCircularQueue<CircularQueueLight>(4); }
TEST(CircularQueueTest, CircularQueueLight8Thread) { TestCircularQueue<CircularQueueLight>(8); }
TEST(CircularQueueTest, CircularQueueLight16Thread) { TestCircularQueue<CircularQueueLight>(16); }

TEST(CircularQueueTest, CircularQueueSync1Thread) { TestCircularQueue<CircularQueueSync>(1); }
TEST(CircularQueueTest, CircularQueueSync2Thread) { TestCircularQueue<CircularQueueSync>(2); }
TEST(CircularQueueTest, CircularQueueSync4Thread) { TestCircularQueue<CircularQueueSync>(4); }
TEST(CircularQueueTest, CircularQueueSync8Thread) { TestCircularQueue<CircularQueueSync>(8); }
TEST(CircularQueueTest, CircularQueueSync16Thread) { TestCircularQueue<CircularQueueSync>(16); }

}  // namespace concurrency
}  // namespace pyc
