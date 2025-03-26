#include "network/io_service_pool/io_service_pool.h"

#include <fmt/base.h>

namespace network {

IOServicePool::IOServicePool(std::size_t size) : io_services_(size) {
    fmt::println("[{}]: Create {} io_context.", __func__, size);

    works_.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        works_.emplace_back(asio::make_work_guard(io_services_[i]));
    }

    threads_.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        threads_.emplace_back([this, i] { io_services_[i].run(); });
    }
}

IOServicePool::~IOServicePool() { fmt::println("[{}]: IOServicePool Exit.", __func__); }

IOServicePool::IOService& IOServicePool::GetIOService() {
    auto& io_service = io_services_[curr_idx_++];
    curr_idx_ %= io_services_.size();
    return io_service;
}

void IOServicePool::Stop() {
    for (auto& io_service : io_services_) {
        io_service.stop();
    }
    for (auto& work : works_) {
        work.reset();
    }
    for (auto& thread : threads_) {
        thread.join();
    }
}

}  // namespace network
