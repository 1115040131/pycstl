#include <chrono>

#include <gtest/gtest.h>

#include "concurrency/thread_safe_hash_table.h"
#include "test/utils.h"

namespace pyc {
namespace concurrency {

using namespace std::literals;

void AddWhileGet(const std::size_t kMaxNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kMaxNum > kThreadNum && (kMaxNum % kThreadNum == 0)) << "kMaxNum 要能被 kThreadNum 均分";

    ThreadSafeHashTable<int, std::shared_ptr<MyClass>> table(97);
    const std::size_t kBlockSize = kMaxNum / kThreadNum;
    bool check[kMaxNum] = {false};

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_FALSE(check[i]);
    }

    std::vector<std::function<void(std::size_t)>> actions;
    // 插入 [0, 2kMaxNum) 的数据
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize * 2;
        for (std::size_t j = 0; j < kBlockSize * 2; j++) {
            table.AddOrUpdateMapping(kStart + j, std::make_shared<MyClass>(kStart + j));
        }
    });
    // 查找并删除 [0, kMaxNum) 的数据
    actions.emplace_back([&](std::size_t i) {
        const std::size_t kStart = i * kBlockSize;
        for (std::size_t j = 0; j < kBlockSize;) {
            auto find_res = table.ValueFor(kStart + j);
            if (find_res) {
                EXPECT_TRUE(find_res->data < static_cast<int>(kMaxNum));
                table.RemoveMapping(find_res->data);
                check[find_res->data] = true;
                j++;
            } else {
                std::this_thread::sleep_for(10ms);
            }
        }
    });
    MultiThreadExecute(kThreadNum, actions);

    for (std::size_t i = 0; i < kMaxNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }

    auto data = table.GetMap();
    EXPECT_EQ(data.size(), kMaxNum);
    for (std::size_t i = kMaxNum; i < kMaxNum * 2; i++) {
        auto iter = data.find(i);
        EXPECT_TRUE(iter != data.end());
        if (iter != data.end()) {
            EXPECT_EQ(iter->second->data, i);
        }
    }
}

TEST(ThreadSafeHashTableTest, ThreadSafeHashTableTest) { AddWhileGet(5000, 8); }

}  // namespace concurrency
}  // namespace pyc
