#include "gate_server/io_service_pool.h"

namespace pyc {
namespace chat {

IOServicePool::IOServicePool(std::size_t size) : io_services_(size) {
    PYC_LOG_INFO("Create {} io_context.", size);

    works_.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        works_.emplace_back(std::make_unique<Work>(io_services_[i]));
    }

    threads_.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        threads_.emplace_back([this, i] { io_services_[i].run(); });
    }
}

IOServicePool::~IOServicePool() {
    Stop();
    PYC_LOG_INFO("IOServicePool Exit.");
}

IOServicePool::IOService& IOServicePool::GetIOService() {
    auto& io_service = io_services_[curr_idx_++];
    curr_idx_ %= io_services_.size();
    return io_service;
}

void IOServicePool::Stop() {
    for (auto& work : works_) {
        // 当 io_service 已经绑定任务时, 需要先停止 io_service, 再释放 work
        work->get_io_context().stop();
        work.reset();
    }
    for (auto& thread : threads_) {
        thread.join();
    }
}

}  // namespace chat
}  // namespace pyc