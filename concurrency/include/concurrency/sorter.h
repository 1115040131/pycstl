#pragma once

#include <atomic>
#include <future>
#include <list>
#include <thread>
#include <vector>

#include "concurrency/thread_safe_stack/thread_safe_stack.h"

namespace pyc {
namespace concurrency {

template <typename T>
class Sorter {
private:
    struct ChunkToSort {
        std::list<T> data;
        std::promise<std::list<T>> promise;
    };

public:
    Sorter() : max_thread_count_(std::thread::hardware_concurrency() - 1), end_of_data_(false) {}

    ~Sorter() {
        end_of_data_ = true;
        for (auto& thread : threads_) {
            thread.join();
        }
    }

    std::list<T> DoSort(std::list<T>& chunk_data) {
        if (chunk_data.empty()) {
            return {};
        }

        std::list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        const T& pivot = *result.begin();
        auto divide_point =
            std::partition(chunk_data.begin(), chunk_data.end(), [&](const T& t) { return t < pivot; });

        ChunkToSort lower_chunk;
        lower_chunk.data.splice(lower_chunk.data.end(), chunk_data, chunk_data.begin(), divide_point);

        std::future<std::list<T>> new_lower = lower_chunk.promise.get_future();
        chunks_.Push(std::move(lower_chunk));
        if (threads_.size() < max_thread_count_) {
            threads_.push_back(std::thread(&Sorter<T>::SortThread, this));
        }

        std::list<T> new_higher(DoSort(chunk_data));
        result.splice(result.end(), new_higher);
        while (new_lower.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            TrySortChunk();
        }
        result.splice(result.begin(), new_lower.get());
        return result;
    }

private:
    void SortThread() {
        while (!end_of_data_) {
            TrySortChunk();
            std::this_thread::yield();
        }
    }

    void TrySortChunk() {
        auto chunk = chunks_.TryPop();
        if (chunk) {
            SortChunk(std::move(chunk.value()));
        }
    }

    void SortChunk(ChunkToSort chunk) { chunk.promise.set_value(DoSort(chunk.data)); }

private:
    ThreadSafeStack<ChunkToSort> chunks_;
    std::vector<std::thread> threads_;
    const unsigned max_thread_count_;
    std::atomic<bool> end_of_data_;
};

template <typename T>
std::list<T> ParallelQuickSort(std::list<T> input) {
    if (input.empty()) {
        return input;
    }
    Sorter<T> sorter;
    return sorter.DoSort(input);
}

}  // namespace concurrency
}  // namespace pyc
