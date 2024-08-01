#include <fmt/format.h>
#include <gtest/gtest.h>

#include "concurrency/lock_free_stack/hazard_pointer_stack.h"
#include "concurrency/lock_free_stack/lock_free_stack.h"
#include "concurrency/lock_free_stack/ref_count_stack.h"
#include "concurrency/test/utils.h"

namespace pyc {
namespace concurrency {

template <typename T>
void PushWhilePop(T& lock_free_stack, const std::size_t kDataNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0))
        << fmt::format("{} 要能被 {} 均分", kDataNum, kThreadNum);

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
    PushWhilePop(lock_free_stack, 10000, 16);
}

TEST(LockFreeStackTest, HazardPointerStackTest) {
    HazardPointerStack<HeapData> lock_free_stack;
    EXPECT_EQ(sizeof(lock_free_stack), 16);
    PushWhilePop(lock_free_stack, 10000, 16);
}

TEST(DISABLED_LockFreeStackTest, RefCountStackTest) {
    RefCountStack<HeapData, false> lock_free_stack;
    EXPECT_EQ(sizeof(lock_free_stack), 16);
    PushWhilePop(lock_free_stack, 100000, 16);
}

TEST(DISABLED_LockFreeStackTest, RefCountStackUseMemoryOrderTest) {
    RefCountStack<HeapData, true> lock_free_stack;
    EXPECT_EQ(sizeof(lock_free_stack), 16);
    PushWhilePop(lock_free_stack, 100000, 16);
}

}  // namespace concurrency
}  // namespace pyc
