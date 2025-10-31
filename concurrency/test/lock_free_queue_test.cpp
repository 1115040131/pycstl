#include <fmt/format.h>
#include <gtest/gtest.h>

#include "concurrency/lock_free_queue/lock_free_queue.h"
#include "concurrency/test/utils.h"

namespace pyc {
namespace concurrency {

template <std::size_t kDataNum, std::size_t kThreadNum>
void PushWhilePop() {
    static_assert(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0), "kDataNum 要能被 kThreadNum 均分");

    LockFreeQueue<HeapData, true> lock_free_queue;
    EXPECT_EQ(sizeof(lock_free_queue), 32);
    bool check[kDataNum] = {false};

    auto push = [&](std::size_t data) { lock_free_queue.Emplace(data); };
    auto pop = [&](std::size_t) {
        auto pop_result = lock_free_queue.Pop();
        if (pop_result) {
            check[pop_result->Data()] = true;
        }
        return pop_result;
    };
    PushWhilePop(kDataNum, kDataNum, kThreadNum, push, pop);

    for (std::size_t i = 0; i < kDataNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }

    EXPECT_FALSE(lock_free_queue.Pop());
}

TEST(DISABLED_LockFreeQueueTest, LockFreeQueueTest) { PushWhilePop<10000, 16>(); }

}  // namespace concurrency
}  // namespace pyc
