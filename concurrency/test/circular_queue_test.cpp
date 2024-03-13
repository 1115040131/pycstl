#include <atomic>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "concurrency/circular_queue_light.h"
#include "concurrency/circular_queue_lock.h"
#include "concurrency/circular_queue_seq.h"
#include "concurrency/circular_queue_sync.h"
#include "test/utils.h"

namespace pyc {
namespace concurrency {

template <template <typename, std::size_t, typename> class QueueType>
void TestCircularQueue(const std::size_t kThreadNum) {
    QueueType<MyClass, 1000, std::allocator<MyClass>> queue;
    constexpr std::size_t kMaxNum = 10000;
    const std::size_t kBlockSize = kMaxNum / kThreadNum;
    std::atomic<std::size_t> count{0};
    bool check[kMaxNum] = {false};

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_FALSE(check[i]);
    }

    std::vector<std::function<void(std::size_t)>> actions;
    // kThreadNum 个线程同时 Push [0, kMaxNum) 的数据
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize; j++) {
            while (!queue.Push(MyClass(kStart + j))) {
            }
        }
    });
    // kThreadNum 个线程同时 Pop [0, kMaxNum) 的数据
    actions.emplace_back([&](std::size_t) {
        for (;;) {
            auto result = queue.Pop();
            if (result.has_value()) {
                EXPECT_TRUE(result.value().data < static_cast<int>(kMaxNum));
                check[result.value().data] = true;
                ++count;
            }
            if (count == kMaxNum) {
                break;
            }
        }
    });
    MultiThreadExecute(kThreadNum, actions);

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
TEST(DISABLED_CircularQueueTest, CircularQueueSeq4Thread) { TestCircularQueue<CircularQueueSeq>(4); }
TEST(DISABLED_CircularQueueTest, CircularQueueSeq8Thread) { TestCircularQueue<CircularQueueSeq>(8); }
TEST(DISABLED_CircularQueueTest, CircularQueueSeq16Thread) { TestCircularQueue<CircularQueueSeq>(16); }

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
