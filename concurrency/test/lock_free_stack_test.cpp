#include <gtest/gtest.h>

#include "concurrency/lock_free_stack/hazard_pointer_stack.h"
#include "concurrency/lock_free_stack/lock_free_stack.h"
#include "test/utils.h"

namespace pyc {
namespace concurrency {

template <typename T>
void PushWhilePop(T& lock_free_stack, const std::size_t kDataNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0))
        << fmt::format("{} 要能被 {} 均分", kDataNum, kThreadNum);

    bool check[kDataNum] = {false};

    auto push = [&](std::size_t data) { lock_free_stack.Emplace(data); };
    auto pop = [&](std::size_t) {
        auto pop_result = lock_free_stack.Pop();
        if (pop_result.has_value()) {
            check[pop_result.value().data] = true;
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
    LockFreeStack<MyClass> lock_free_stack;
    PushWhilePop(lock_free_stack, 10000, 16);
}

TEST(LockFreeStackTest, LockFreeStackHazardPointerTest) {
    HazardPointerStack<MyClass> lock_free_stack;
    PushWhilePop(lock_free_stack, 10000, 16);
}

}  // namespace concurrency
}  // namespace pyc
