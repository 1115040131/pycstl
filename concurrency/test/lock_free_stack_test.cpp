#include <fmt/format.h>
#include <gtest/gtest.h>

#include "concurrency/lock_free_stack/hazard_pointer_stack.h"
#include "concurrency/lock_free_stack/lock_free_stack.h"
#include "concurrency/lock_free_stack/ref_count_stack.h"
#include "concurrency/test/utils.h"

namespace pyc {
namespace concurrency {

template <std::size_t kDataNum, std::size_t kThreadNum, typename T>
void PushWhilePop(T& lock_free_stack) {
    static_assert(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0), "kDataNum 要能被 kThreadNum 均分");

    bool check[kDataNum] = {false};

    auto push = [&](std::size_t data) { lock_free_stack.Emplace(static_cast<int>(data)); };
    auto pop = [&](std::size_t) {
        auto pop_result = lock_free_stack.Pop();
        if (pop_result.has_value()) {
            check[pop_result.value().Data()] = true;
        }
        return pop_result.has_value();
    };
    PushWhilePop(kDataNum, kDataNum, kThreadNum, push, pop);

    for (std::size_t i = 0; i < kDataNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }

    EXPECT_FALSE(lock_free_stack.Pop());
}

TEST(LockFreeStackTest, LockFreeStackTest) {
    LockFreeStack<HeapData> lock_free_stack;
    EXPECT_EQ(sizeof(lock_free_stack), 24);
    PushWhilePop<10000, 16>(lock_free_stack);
}

TEST(LockFreeStackTest, HazardPointerStackTest) {
    HazardPointerStack<HeapData> lock_free_stack;
    EXPECT_EQ(sizeof(lock_free_stack), 16);
    PushWhilePop<10000, 16>(lock_free_stack);
}

TEST(DISABLED_LockFreeStackTest, RefCountStackTest) {
    RefCountStack<HeapData, false> lock_free_stack;
    EXPECT_EQ(sizeof(lock_free_stack), 16);
    PushWhilePop<100000, 16>(lock_free_stack);
}

TEST(DISABLED_LockFreeStackTest, RefCountStackUseMemoryOrderTest) {
    RefCountStack<HeapData, true> lock_free_stack;
    EXPECT_EQ(sizeof(lock_free_stack), 16);
    PushWhilePop<100000, 16>(lock_free_stack);
}

}  // namespace concurrency
}  // namespace pyc
