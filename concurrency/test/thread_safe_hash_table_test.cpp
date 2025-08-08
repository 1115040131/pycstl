#include <chrono>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "concurrency/test/utils.h"
#include "concurrency/thread_safe_hash_table/thread_safe_hash_table.h"

namespace pyc {
namespace concurrency {

using namespace std::literals;

void AddWhileRemove(const std::size_t kDataNum, const std::size_t kThreadNum) {
    ASSERT_TRUE(kDataNum > kThreadNum && (kDataNum % kThreadNum == 0))
        << fmt::format("{} 要能被 {} 均分", kDataNum, kThreadNum);

    ThreadSafeHashTable<int, std::shared_ptr<MyClass>> table(97);
    bool check[kDataNum] = {false};

    // 插入 [0, 2kDataNum) 的数据
    auto add = [&](std::size_t data) { table.AddOrUpdateMapping(data, std::make_shared<MyClass>(data)); };
    // 查找并删除 [0, kDataNum) 的数据
    auto remove = [&](std::size_t data) {
        auto find_result = table.ValueFor(data);
        if (find_result) {
            table.RemoveMapping(find_result->data);
            check[find_result->data] = true;
        }
        return find_result;
    };
    PushWhilePop(kDataNum * 2, kDataNum, kThreadNum, add, remove);

    for (std::size_t i = 0; i < kDataNum; i++) {
        EXPECT_TRUE(check[i]) << i;
    }

    auto data = table.GetMap();
    EXPECT_EQ(data.size(), kDataNum);
    for (std::size_t i = kDataNum; i < kDataNum * 2; i++) {
        auto iter = data.find(i);
        EXPECT_TRUE(iter != data.end());
        if (iter != data.end()) {
            EXPECT_EQ(iter->second->data, i);
        }
    }
}

TEST(ThreadSafeHashTableTest, AddWhileRemoveTest) { AddWhileRemove(5000, 8); }

}  // namespace concurrency
}  // namespace pyc
