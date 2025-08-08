#include <fmt/format.h>
#include <gtest/gtest.h>

#include "concurrency/test/utils.h"
#include "concurrency/thread_safe_list/double_push_list.h"
#include "concurrency/thread_safe_list/thread_safe_list.h"

namespace pyc {
namespace concurrency {

template <typename T>
void PushFrontWhileFind(T& thread_safe_list, const std::size_t kDataNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0))
        << fmt::format("{} 要能被 {} 均分", kDataNum, kThreadNum);

    bool check[kDataNum] = {false};

    auto push_front = [&](std::size_t data) { thread_safe_list.PushFront(MyClass{static_cast<int>(data)}); };
    auto find = [&](std::size_t data) {
        auto find_result =
            thread_safe_list.FindFirstIf([&](const MyClass& item) { return item.data == static_cast<int>(data); });
        if (find_result) {
            check[find_result->data] = true;
        }
        return find_result;
    };
    PushWhilePop(kDataNum, kDataNum, kThreadNum, push_front, find);

    for (std::size_t i = 0; i < kDataNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }
}

template <typename T>
void PushFrontWhileRemove(T& thread_safe_list, const std::size_t kDataNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0))
        << fmt::format("{} 要能被 {} 均分", kDataNum, kThreadNum);

    bool check[kDataNum] = {false};

    auto push_front = [&](std::size_t data) { thread_safe_list.PushFront(MyClass{static_cast<int>(data)}); };
    auto remove = [&](std::size_t data) {
        auto predicate = [&](const MyClass& item) { return item.data == static_cast<int>(data); };
        auto find_result = thread_safe_list.FindFirstIf(predicate);
        if (find_result) {
            thread_safe_list.RemoveIf(predicate);
            check[find_result->data] = true;
        }
        return find_result;
    };
    PushWhilePop(kDataNum, kDataNum, kThreadNum, push_front, remove);

    for (std::size_t i = 0; i < kDataNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }

    int cnt = 0;
    thread_safe_list.ForEach([&](const MyClass&) { cnt++; });
    EXPECT_EQ(cnt, 0);
}

void DoublePushWhileRemove(const std::size_t kDataNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kDataNum >= kThreadNum && (kDataNum % kThreadNum == 0))
        << fmt::format("{} 要能被 {} 均分", kDataNum, kThreadNum);

    DoublePushList<MyClass> double_push_list;
    bool check[2 * kDataNum] = {false};

    auto push = [&](std::size_t data) {
        double_push_list.PushFront(MyClass{static_cast<int>(data)});
        double_push_list.PushBack(MyClass{static_cast<int>(data + kDataNum)});
    };
    auto remove = [&](std::size_t data) {
        auto predicate = [=](const MyClass& item) { return item.data == static_cast<int>(data + kDataNum / 2); };
        auto find_result = double_push_list.FindFirstIf(predicate);
        if (find_result) {
            double_push_list.RemoveIf(predicate);
            check[find_result->data] = true;
        }
        return find_result;
    };
    PushWhilePop(kDataNum, kDataNum, kThreadNum, push, remove);

    for (std::size_t i = 0; i < kDataNum * 2; i++) {
        if (i < kDataNum / 2 || i >= kDataNum / 2 + kDataNum) {
            EXPECT_FALSE(check[i]) << i;
        } else {
            EXPECT_TRUE(check[i]) << i;
        }
    }

    int cnt = 0;
    double_push_list.ForEach([&](const MyClass&) { cnt++; });
    EXPECT_EQ(cnt, kDataNum);
}

TEST(ThreadSafeListTest, FindWhilePushTest) {
    ThreadSafeList<MyClass> thread_safe_list;
    PushFrontWhileFind(thread_safe_list, 2000, 16);

    DoublePushList<MyClass> double_push_list;
    PushFrontWhileFind(double_push_list, 2000, 16);
}

TEST(ThreadSafeListTest, PushFrontWhileRemoveTest) {
    ThreadSafeList<MyClass> thread_safe_list;
    PushFrontWhileRemove(thread_safe_list, 2000, 16);

    DoublePushList<MyClass> double_push_list;
    PushFrontWhileRemove(double_push_list, 2000, 16);
}

TEST(ThreadSafeListTest, DoublePushWhileRemoveTest) { DoublePushWhileRemove(2000, 16); }

}  // namespace concurrency
}  // namespace pyc
