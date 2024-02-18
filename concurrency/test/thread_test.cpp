#include <algorithm>
#include <atomic>
#include <numeric>
#include <thread>
#include <vector>

#include <fmt/core.h>
#include <gtest/gtest.h>

#include "common/noncopyable.h"

namespace pyc {
namespace concurrency {

static std::atomic<size_t> s_count{0};

static constexpr size_t kLoopTime = 100000;

void Loop() {
    for (size_t i = 0; i < kLoopTime; i++) {
        s_count++;
    }
}

TEST(ThreadTest, ThreadOwnership) {
    {
        std::thread t1{Loop};
        std::thread t2{Loop};

        // t2 运行中, 对其赋值会导致原线程 terminal
        EXPECT_DEATH(t2 = std::move(t1), ".*");

        t1.join();
        t2.join();
    }

    EXPECT_DEATH({ std::thread t{Loop}; }, ".*");

    s_count = 0;
    {
        std::thread t{Loop};
        t.join();
    }
    EXPECT_EQ(s_count, kLoopTime);

    s_count = 0;
    { std::jthread jt{Loop}; }
    EXPECT_EQ(s_count, kLoopTime);

    s_count = 0;
    {
        std::jthread jt1{Loop};
        std::jthread jt2{Loop};
    }
    EXPECT_EQ(s_count, 2 * kLoopTime);
}

template <typename Iterator, typename T>
void AccumulateBlock(Iterator first, Iterator last, T& init) {
    init = std::accumulate(first, last, init);
}

template <typename Iterator, typename T>
T ParallelAccumulate(Iterator first, Iterator last, T init) {
    const std::size_t length = std::distance(first, last);
    if (length == 0) {
        return init;
    }

    constexpr std::size_t kMinPerThread = 25;
    const std::size_t max_threads = (length + kMinPerThread - 1) / kMinPerThread;
    const std::size_t hardware_threads = std::thread::hardware_concurrency();
    const std::size_t num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    const std::size_t block_size = length / num_threads;

    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);

    Iterator block_start = first;
    for (std::size_t i = 0; i < (num_threads - 1); ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(AccumulateBlock<Iterator, T>, block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }
    AccumulateBlock(block_start, last, results[num_threads - 1]);

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    return std::accumulate(results.begin(), results.end(), init);
}

TEST(ThreadTest, ParallelAccumulate) {
    std::vector<int> vec(10000);
    std::iota(vec.begin(), vec.end(), 0);
    int sum = 0;
    EXPECT_EQ(ParallelAccumulate(vec.begin(), vec.end(), sum), 49995000);
}

}  // namespace concurrency
}  // namespace pyc