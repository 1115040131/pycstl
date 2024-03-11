#include <fmt/core.h>
#include <gtest/gtest.h>

#include "concurrency/thread_pool.h"

namespace pyc {
namespace concurrency {

using namespace std::literals::chrono_literals;

TEST(ThreadPoolTest, CommitFunction) {
    int m = 0;
    ThreadPool::GetInstance().Commit([](int& m) { m = 1024; }, m);
    std::this_thread::sleep_for(10ms);
    EXPECT_EQ(m, 0);

    ThreadPool::GetInstance().Commit([](int& m) { m = 1024; }, std::ref(m));
    std::this_thread::sleep_for(10ms);
    EXPECT_EQ(m, 1024);
}

}  // namespace concurrency
}  // namespace pyc