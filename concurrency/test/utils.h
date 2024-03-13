#pragma once

#include <functional>
#include <thread>
#include <vector>

namespace pyc {
namespace concurrency {

struct MyClass {
    int data;
};

/// @brief 对于 actions 中的每个函数, 启动 kThreadNum 个线程执行
inline void MultiThreadExecute(const std::size_t kThreadNum,
                        const std::vector<std::function<void(std::size_t)>>& actions) {
    std::vector<std::vector<std::jthread>> executors(actions.size());
    for (auto& executor : executors) {
        executor.reserve(kThreadNum);
    }

    for (std::size_t i = 0; i < actions.size(); i++) {
        for (std::size_t j = 0; j < kThreadNum; j++) {
            executors[i].emplace_back(actions[i], j);
        }
    }
}

}  // namespace concurrency
}  // namespace pyc
