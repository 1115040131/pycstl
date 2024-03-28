#pragma once

#include <algorithm>
#include <atomic>
#include <future>
#include <thread>
#include <vector>

namespace pyc {
namespace concurrency {

inline std::size_t GetTaskThreadNum(std::size_t length) {
    constexpr std::size_t kMinPerThread = 25;
    const std::size_t kMaxThread = (length + kMinPerThread - 1) / kMinPerThread;
    const std::size_t kHardwareThread = std::thread::hardware_concurrency();
    return std::min(std::max(kHardwareThread, 2ul), kMaxThread);
}

template <std::input_iterator Iterator, typename Predicate>
void ParallelForEach(Iterator first, Iterator last, Predicate p) {
    const std::size_t kLength = std::distance(first, last);
    if (kLength == 0) {
        return;
    }
    const std::size_t kThreadNum = GetTaskThreadNum(kLength);
    const std::size_t kBlockSize = kLength / kThreadNum;
    std::vector<std::future<void>> futures(kThreadNum - 1);
    std::vector<std::jthread> threads(kThreadNum - 1);
    Iterator block_start = first;
    for (std::size_t i = 0; i < kThreadNum - 1; i++) {
        Iterator block_end = block_start;
        std::advance(block_end, kBlockSize);
        std::packaged_task<void()> task([=]() { std::for_each(block_start, block_end, p); });
        futures[i] = task.get_future();
        threads[i] = std::jthread(std::move(task));
        block_start = block_end;
    }
    std::for_each(block_start, last, p);
    for (std::size_t i = 0; i < kThreadNum - 1; i++) {
        futures[i].get();
    }
}

template <std::input_iterator Iterator, typename Match>
Iterator ParallelFind(Iterator first, Iterator last, Match match) {
    const std::size_t kLength = std::distance(first, last);
    if (kLength == 0) {
        return last;
    }
    const std::size_t kThreadNum = GetTaskThreadNum(kLength);
    const std::size_t kBlockSize = kLength / kThreadNum;
    std::promise<Iterator> result;
    std::atomic<bool> found = false;
    std::vector<std::jthread> threads(kThreadNum - 1);
    {
        auto find_block = [&](Iterator begin, Iterator end) {
            while (begin != end && !found) {
                if (*begin == match) {
                    found = true;
                    result.set_value(begin);
                    return;
                }
                ++begin;
            }
        };

        Iterator block_start = first;
        for (std::size_t i = 0; i < kThreadNum - 1; i++) {
            Iterator block_end = block_start;
            std::advance(block_end, kBlockSize);
            threads[i] = std::jthread(find_block, block_start, block_end);
            block_start = block_end;
        }
        find_block(block_start, last);
    }
    if (!found) {
        return last;
    }
    return result.get_future().get();
}

}  // namespace concurrency
}  // namespace pyc
