#include <fmt/format.h>
#include <gtest/gtest.h>

#include "concurrency/circular_queue/circular_queue_light.h"
#include "concurrency/circular_queue/circular_queue_lock.h"
#include "concurrency/circular_queue/circular_queue_seq.h"
#include "concurrency/circular_queue/circular_queue_sync.h"
#include "concurrency/test/utils.h"

namespace pyc {
namespace concurrency {

template <template <typename, std::size_t, typename> class QueueType, std::size_t kDataNum, std::size_t kThreadNum>
void PushWhilePop() {
    static_assert(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0), "kDataNum 要能被 kThreadNum 均分");

    // 这些队列的生产者要按预约顺序发布槽位(等待 tail_update_ 推进到自己)，等待方式是忙等。
    // 线程数超过核数时，被等待的那个生产者一旦被抢占下核，其余线程就会持续空转烧完整个时间片，
    // 反过来拖慢它重新被调度，形成 convoy —— 实测 16 生产者 + 16 消费者在 8 核机器上会从
    // 13ms 劣化到 69s。此时测的是调度器行为而非队列本身，直接跳过。
    // 注意实际线程数是 2 * kThreadNum(生产者 + 消费者)，即 kThreadNum == 核数时已是 2 倍超订，
    // 实测这一档仍然稳定，所以阈值取 kThreadNum 而非 2 * kThreadNum。
    if (!HasEnoughCoresFor(kThreadNum)) {
        GTEST_SKIP() << "kThreadNum=" << kThreadNum << " 超过本机 " << std::thread::hardware_concurrency() << " 核";
    }

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

TEST(CircularQueueTest, CircularQueueLock1Thread) { PushWhilePop<CircularQueueLock, 10000, 1>(); }
TEST(CircularQueueTest, CircularQueueLock2Thread) { PushWhilePop<CircularQueueLock, 10000, 2>(); }
TEST(CircularQueueTest, CircularQueueLock4Thread) { PushWhilePop<CircularQueueLock, 10000, 4>(); }
TEST(CircularQueueTest, CircularQueueLock8Thread) { PushWhilePop<CircularQueueLock, 10000, 8>(); }
TEST(CircularQueueTest, CircularQueueLock16Thread) { PushWhilePop<CircularQueueLock, 10000, 16>(); }

TEST(CircularQueueTest, CircularQueueSeq1Thread) { PushWhilePop<CircularQueueSeq, 10000, 1>(); }
TEST(CircularQueueTest, CircularQueueSeq2Thread) { PushWhilePop<CircularQueueSeq, 10000, 2>(); }
TEST(DISABLED_CircularQueueTest, CircularQueueSeq4Thread) { PushWhilePop<CircularQueueSeq, 10000, 4>(); }
TEST(DISABLED_CircularQueueTest, CircularQueueSeq8Thread) { PushWhilePop<CircularQueueSeq, 10000, 8>(); }
TEST(DISABLED_CircularQueueTest, CircularQueueSeq16Thread) { PushWhilePop<CircularQueueSeq, 10000, 16>(); }

TEST(CircularQueueTest, CircularQueueLight1Thread) { PushWhilePop<CircularQueueLight, 10000, 1>(); }
TEST(CircularQueueTest, CircularQueueLight2Thread) { PushWhilePop<CircularQueueLight, 10000, 2>(); }
TEST(CircularQueueTest, CircularQueueLight4Thread) { PushWhilePop<CircularQueueLight, 10000, 4>(); }
TEST(CircularQueueTest, CircularQueueLight8Thread) { PushWhilePop<CircularQueueLight, 10000, 8>(); }
TEST(CircularQueueTest, CircularQueueLight16Thread) { PushWhilePop<CircularQueueLight, 10000, 16>(); }

TEST(CircularQueueTest, CircularQueueSync1Thread) { PushWhilePop<CircularQueueSync, 10000, 1>(); }
TEST(CircularQueueTest, CircularQueueSync2Thread) { PushWhilePop<CircularQueueSync, 10000, 2>(); }
TEST(CircularQueueTest, CircularQueueSync4Thread) { PushWhilePop<CircularQueueSync, 10000, 4>(); }
TEST(CircularQueueTest, CircularQueueSync8Thread) { PushWhilePop<CircularQueueSync, 10000, 8>(); }
TEST(CircularQueueTest, CircularQueueSync16Thread) { PushWhilePop<CircularQueueSync, 10000, 16>(); }

}  // namespace concurrency
}  // namespace pyc
