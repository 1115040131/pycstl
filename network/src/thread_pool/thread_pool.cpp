#include "network/thread_pool/thread_pool.h"

#include <fmt/printf.h>

namespace network {

ThreadPool::ThreadPool(std::size_t size) {
    threads_.reserve(size);

    for (std::size_t i = 0; i < size; ++i) {
        threads_.emplace_back([this] { io_service_.run(); });
    }
}

ThreadPool::~ThreadPool() { fmt::println("[{}]: ThreadPool Exit.", __func__); }

ThreadPool::IOService& ThreadPool::GetIOService() { return io_service_; }

void ThreadPool::Stop() {
    io_service_.stop();
    work_.reset();
    for (auto& thread : threads_) {
        thread.join();
    }
}

}  // namespace network
