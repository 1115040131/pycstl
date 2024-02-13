#include "network/thread_pool_server/pool.h"

#include <fmt/printf.h>

namespace network {

ThreadPool::ThreadPool(size_t size) {
    threads_.reserve(size);

    for (size_t i = 0; i < size; ++i) {
        threads_.emplace_back([this] { io_service_.run(); });
    }
}

ThreadPool::~ThreadPool() { fmt::println("[{}]: ThreadPool Exit.", __func__); }

ThreadPool::IOService& ThreadPool::GetIOService() { return io_service_; }

void ThreadPool::Stop() {
    work_.reset();
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            if (thread.get_id() != std::this_thread::get_id()) {
                thread.join();
            } else {
                thread.detach();  // 确保不会从线程内部尝试 join 自己。
            }
        }
    }
}

}  // namespace network
