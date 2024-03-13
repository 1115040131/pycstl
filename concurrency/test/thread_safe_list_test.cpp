#include <gtest/gtest.h>

#include "concurrency/double_push_list.h"
#include "concurrency/thread_safe_list.h"
#include "test/utils.h"

namespace pyc {
namespace concurrency {

template <typename T>
void FindWhilePushFront(const std::size_t kMaxNum, const std::size_t kThreadNum, T& thread_safe_list) {
    ASSERT_TRUE(kMaxNum >= kThreadNum && (kMaxNum % kThreadNum == 0)) << "kMaxNum 要能被 kThreadNum 均分";

    const std::size_t kBlockSize = kMaxNum / kThreadNum;
    bool check[kMaxNum] = {false};

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_FALSE(check[i]);
    }

    std::vector<std::function<void(std::size_t)>> actions;
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize; j++) {
            thread_safe_list.PushFront(MyClass{static_cast<int>(kStart + j)});
        }
    });
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize;) {
            auto find_res = thread_safe_list.FindFirstIf(
                [&](const MyClass& item) { return item.data == static_cast<int>(kStart + j); });
            if (find_res != nullptr) {
                EXPECT_EQ(find_res->data, static_cast<int>(kStart + j));
                check[find_res->data] = true;
                j++;
            }
        }
    });
    MultiThreadExecute(kThreadNum, actions);

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }
}

template <typename T>
void RemoveWhilePush(const std::size_t kMaxNum, const std::size_t kThreadNum, T& thread_safe_list) {
    ASSERT_TRUE(kMaxNum >= kThreadNum && (kMaxNum % kThreadNum == 0)) << "kMaxNum 要能被 kThreadNum 均分";

    const std::size_t kBlockSize = kMaxNum / kThreadNum;

    std::vector<std::function<void(std::size_t)>> actions;
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize; j++) {
            thread_safe_list.PushFront(MyClass{static_cast<int>(kStart + j)});
        }
    });
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize;) {
            auto predicate = [&](const MyClass& item) { return item.data == static_cast<int>(kStart + j); };
            if (thread_safe_list.FindFirstIf(predicate)) {
                thread_safe_list.RemoveIf(predicate);
                j++;
            }
        }
    });
    MultiThreadExecute(kThreadNum, actions);

    int cnt = 0;
    thread_safe_list.ForEach([&](const MyClass&) { cnt++; });
    EXPECT_EQ(cnt, 0);
}

void RemoveWhileDoublePush(const std::size_t kMaxNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kMaxNum >= kThreadNum && (kMaxNum % kThreadNum == 0)) << "kMaxNum 要能被 kThreadNum 均分";

    DoublePushList<MyClass> double_push_list;
    const std::size_t kBlockSize = kMaxNum / kThreadNum;

    std::vector<std::function<void(std::size_t)>> actions;
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize; j++) {
            double_push_list.PushFront(MyClass{static_cast<int>(kStart + j)});
        }
    });
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = kMaxNum + i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize; j++) {
            double_push_list.PushBack(MyClass{static_cast<int>(kStart + j)});
        }
    });
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize * 2;
        for (std::size_t j = 0; j < kBlockSize * 2;) {
            auto predicate = [&](const MyClass& item) { return item.data == static_cast<int>(kStart + j); };
            if (double_push_list.FindFirstIf(predicate)) {
                double_push_list.RemoveIf(predicate);
                j++;
            }
        }
    });
    MultiThreadExecute(kThreadNum, actions);

    int cnt = 0;
    double_push_list.ForEach([&](const MyClass&) { cnt++; });
    EXPECT_EQ(cnt, 0);
}

TEST(ThreadSafeListTest, FindWhilePushTest) {
    ThreadSafeList<MyClass> thread_safe_list;
    FindWhilePushFront(2000, 16, thread_safe_list);

    DoublePushList<MyClass> double_push_list;
    FindWhilePushFront(2000, 16, double_push_list);
}

TEST(ThreadSafeListTest, RemoveWhilePushTest) {
    ThreadSafeList<MyClass> thread_safe_list;
    RemoveWhilePush(2000, 16, thread_safe_list);

    DoublePushList<MyClass> double_push_list;
    RemoveWhilePush(2000, 16, double_push_list);
}

TEST(ThreadSafeListTest, RemoveWhileDoublePushTest) { RemoveWhileDoublePush(2000, 16); }

}  // namespace concurrency
}  // namespace pyc
