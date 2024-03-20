#include <gtest/gtest.h>

#include "concurrency/circular_queue/circular_queue_light.h"
#include "concurrency/circular_queue/circular_queue_lock.h"
#include "concurrency/circular_queue/circular_queue_seq.h"
#include "concurrency/circular_queue/circular_queue_sync.h"
#include "test/utils.h"

namespace pyc {
namespace concurrency {

template <template <typename, std::size_t, typename> class QueueType>
void PushWhilePop(const std::size_t kDataNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0))
        << fmt::format("{} 要能被 {} 均分", kDataNum, kThreadNum);

    QueueType<MyClass, 1000, std::allocator<MyClass>> queue;
    bool check[kDataNum] = {false};

    auto push = [&](std::size_t data) {
        while (!queue.Push(MyClass(data))) {
        }
    };
    auto pop = [&](std::size_t) {
        auto result = queue.Pop();
        if (result.has_value()) {
            check[result.value().data] = true;
        }
        return result;
    };
    PushWhilePop(kDataNum, kDataNum, kThreadNum, push, pop);

    for (std::size_t i = 0; i < kDataNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }
}

TEST(CircularQueueTest, CircularQueueLock1Thread) { PushWhilePop<CircularQueueLock>(10000, 1); }
TEST(CircularQueueTest, CircularQueueLock2Thread) { PushWhilePop<CircularQueueLock>(10000, 2); }
TEST(CircularQueueTest, CircularQueueLock4Thread) { PushWhilePop<CircularQueueLock>(10000, 4); }
TEST(CircularQueueTest, CircularQueueLock8Thread) { PushWhilePop<CircularQueueLock>(10000, 8); }
TEST(CircularQueueTest, CircularQueueLock16Thread) { PushWhilePop<CircularQueueLock>(10000, 16); }

TEST(CircularQueueTest, CircularQueueSeq1Thread) { PushWhilePop<CircularQueueSeq>(10000, 1); }
TEST(CircularQueueTest, CircularQueueSeq2Thread) { PushWhilePop<CircularQueueSeq>(10000, 2); }
TEST(DISABLED_CircularQueueTest, CircularQueueSeq4Thread) { PushWhilePop<CircularQueueSeq>(10000, 4); }
TEST(DISABLED_CircularQueueTest, CircularQueueSeq8Thread) { PushWhilePop<CircularQueueSeq>(10000, 8); }
TEST(DISABLED_CircularQueueTest, CircularQueueSeq16Thread) { PushWhilePop<CircularQueueSeq>(10000, 16); }

TEST(CircularQueueTest, CircularQueueLight1Thread) { PushWhilePop<CircularQueueLight>(10000, 1); }
TEST(CircularQueueTest, CircularQueueLight2Thread) { PushWhilePop<CircularQueueLight>(10000, 2); }
TEST(CircularQueueTest, CircularQueueLight4Thread) { PushWhilePop<CircularQueueLight>(10000, 4); }
TEST(CircularQueueTest, CircularQueueLight8Thread) { PushWhilePop<CircularQueueLight>(10000, 8); }
TEST(CircularQueueTest, CircularQueueLight16Thread) { PushWhilePop<CircularQueueLight>(10000, 16); }

TEST(CircularQueueTest, CircularQueueSync1Thread) { PushWhilePop<CircularQueueSync>(10000, 1); }
TEST(CircularQueueTest, CircularQueueSync2Thread) { PushWhilePop<CircularQueueSync>(10000, 2); }
TEST(CircularQueueTest, CircularQueueSync4Thread) { PushWhilePop<CircularQueueSync>(10000, 4); }
TEST(CircularQueueTest, CircularQueueSync8Thread) { PushWhilePop<CircularQueueSync>(10000, 8); }
TEST(CircularQueueTest, CircularQueueSync16Thread) { PushWhilePop<CircularQueueSync>(10000, 16); }

}  // namespace concurrency
}  // namespace pyc
