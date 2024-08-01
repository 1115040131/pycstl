#pragma once

#include <functional>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace pyc {
namespace concurrency {

struct MyClass {
    int data;

    int Data() const { return data; }
};

class HeapData {
public:
    // Constructor
    HeapData(int data) { data_ = new int(data); }

    // Destructor
    ~HeapData() { delete data_; }

    // Copy constructor
    HeapData(const HeapData& other) {
        if (other.data_) {
            data_ = new int(*(other.data_));
        } else {
            data_ = nullptr;
        }
    }

    // Copy assignment operator
    HeapData& operator=(const HeapData& other) {
        if (this != &other) {
            delete data_;
            data_ = nullptr;

            if (other.data_) {
                data_ = new int(*(other.data_));
            }
        }
        return *this;
    }

    // Move constructor
    HeapData(HeapData&& other) noexcept : data_(other.data_) { other.data_ = nullptr; }

    // Move assignment operator
    HeapData& operator=(HeapData&& other) noexcept {
        if (this != &other) {
            delete data_;
            data_ = other.data_;
            other.data_ = nullptr;
        }
        return *this;
    }

    // Accessor
    int Data() const { return *data_; }

private:
    int* data_;
};

/// @brief 对于 actions 中的每个函数, 启动 kThreadNum 个线程执行
inline void MultiThreadExecute(const std::size_t kThreadNum,
                               const std::vector<std::function<void(std::size_t)>>& actions) {
    std::vector<std::vector<std::jthread>> executors(actions.size());
    for (auto& executor : executors) {
        executor.reserve(kThreadNum);
    }

    for (std::size_t i = 0; i < actions.size(); i++) {
        for (std::size_t thread_idx = 0; thread_idx < kThreadNum; thread_idx++) {
            executors[i].emplace_back(actions[i], thread_idx);
        }
    }
}

template <typename PushFunction>
void MultiThreadPush(const std::size_t kPushNum, const std::size_t kThreadNum, PushFunction push) {
    std::vector<std::function<void(std::size_t)>> actions;

    const std::size_t kPushBlock = kPushNum / kThreadNum;  // 分块大小
    actions.emplace_back([&](std::size_t thread_idx) {
        const std::size_t kStart = thread_idx * kPushBlock;  // 分块起始位置
        for (std::size_t push_idx = 0; push_idx < kPushBlock; push_idx++) {
            push(kStart + push_idx);
        }
    });

    MultiThreadExecute(kThreadNum, actions);
}

template <typename PopFunction>
void MultiThreadPop(const std::size_t kPopNum, const std::size_t kThreadNum, PopFunction pop) {
    std::vector<std::function<void(std::size_t)>> actions;

    const std::size_t kPopBlock = kPopNum / kThreadNum;
    actions.emplace_back([&](std::size_t thread_idx) {
        const std::size_t kStart = thread_idx * kPopBlock;
        for (std::size_t pop_idx = 0; pop_idx < kPopBlock;) {
            // 有些 pop 接口可能返回 false 所以要确认 pop 到数据后再迭代
            if (pop(kStart + pop_idx)) {
                pop_idx++;
            }
        }
    });

    MultiThreadExecute(kThreadNum, actions);
}

template <typename PushFunction, typename PopFunction>
void PushWhilePop(const std::size_t kPushNum, const std::size_t kPopNum, const std::size_t kThreadNum,
                  PushFunction push, PopFunction pop) {
    std::vector<std::function<void(std::size_t)>> actions;

    const std::size_t kPushBlock = kPushNum / kThreadNum;  // 分块大小
    actions.emplace_back([&](std::size_t thread_idx) {
        const std::size_t kStart = thread_idx * kPushBlock;  // 分块起始位置
        for (std::size_t push_idx = 0; push_idx < kPushBlock; push_idx++) {
            push(kStart + push_idx);
        }
    });

    const std::size_t kPopBlock = kPopNum / kThreadNum;
    actions.emplace_back([&](std::size_t thread_idx) {
        const std::size_t kStart = thread_idx * kPopBlock;
        for (std::size_t pop_idx = 0; pop_idx < kPopBlock;) {
            // 有些 pop 接口可能返回 false 所以要确认 pop 到数据后再迭代
            if (pop(kStart + pop_idx)) {
                pop_idx++;
            }
        }
    });

    MultiThreadExecute(kThreadNum, actions);
}

}  // namespace concurrency
}  // namespace pyc
