#include <vector>

#include <fmt/base.h>
#include <gtest/gtest.h>

#include "concurrency/parallel_algorithm.h"
#include "concurrency/thread_pool/simple_thread_pool.h"
#include "concurrency/thread_pool/steal_thread_pool.h"
#include "concurrency/thread_pool/thread_pool.h"

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

/// @brief 轮询线程池实现并行 for_each
template <std::input_iterator Iterator, typename Predicate>
void SimpleForEach(Iterator first, Iterator last, Predicate p) {
    const std::size_t kLength = std::distance(first, last);
    if (kLength == 0) {
        return;
    }
    const std::size_t kThreadNum = GetTaskThreadNum(kLength);
    const std::size_t kBlockSize = kLength / kThreadNum;
    Iterator block_start = first;
    for (std::size_t i = 0; i < kThreadNum - 1; ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, kBlockSize);
        SimpleThreadPool::GetInstance().Commit([=]() { std::for_each(block_start, block_end, p); });
        block_start = block_end;
    }
    std::for_each(block_start, last, p);
}

TEST(ThreadPoolTest, SimpleThreadPoolTest) {
    std::vector<int> origin_data;
    for (std::size_t i = 0; i < 26; i++) {
        origin_data.push_back(i);
    }
    auto vec1 = origin_data;
    // 轮询线程池性能开销较大, 使用完后手动停止
    SimpleThreadPool::GetInstance().Start();
    SimpleForEach(vec1.begin(), vec1.end(), [](int& i) { i *= 2; });
    std::this_thread::sleep_for(10ms);
    SimpleThreadPool::GetInstance().Stop();
    auto vec2 = origin_data;
    std::for_each(vec2.begin(), vec2.end(), [](int& i) { i *= 2; });
    EXPECT_EQ(vec1, vec2);
}

/// @brief 任务窃取线程池实现并行 for_each
template <std::input_iterator Iterator, typename Predicate>
void StealForEach(Iterator first, Iterator last, Predicate p) {
    const std::size_t kLength = std::distance(first, last);
    if (kLength == 0) {
        return;
    }
    const std::size_t kThreadNum = GetTaskThreadNum(kLength);
    const std::size_t kBlockSize = kLength / kThreadNum;
    std::vector<std::future<void>> futures(kThreadNum - 1);
    Iterator block_start = first;
    for (std::size_t i = 0; i < kThreadNum - 1; ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, kBlockSize);
        futures[i] = StealThreadPool::GetInstance().Commit([=]() { std::for_each(block_start, block_end, p); });
        block_start = block_end;
    }
    std::for_each(block_start, last, p);
    for (std::size_t i = 0; i < kThreadNum - 1; i++) {
        futures[i].get();
    }
}

TEST(ThreadPoolTest, StealThreadPoolTest) {
    std::vector<int> origin_data;
    for (std::size_t i = 0; i < 26; i++) {
        origin_data.push_back(i);
    }
    auto vec1 = origin_data;
    // 轮询线程池性能开销较大, 使用完后手动停止
    StealThreadPool::GetInstance().Start();
    StealForEach(vec1.begin(), vec1.end(), [](int& i) { i *= 2; });
    StealThreadPool::GetInstance().Stop();
    auto vec2 = origin_data;
    std::for_each(vec2.begin(), vec2.end(), [](int& i) { i *= 2; });
    EXPECT_EQ(vec1, vec2);
}

}  // namespace concurrency
}  // namespace pyc