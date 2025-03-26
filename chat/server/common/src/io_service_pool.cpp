#include "chat/server/common/io_service_pool.h"

#include "logger/logger.h"

namespace pyc {
namespace chat {

static Logger g_logger("IOServicePool");

IOServicePool::IOServicePool(std::size_t size) : io_services_(size) {
    g_logger.info("Create {} io_context.", size);

    works_.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        works_.emplace_back(boost::asio::make_work_guard(io_services_[i]));
    }

    threads_.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        threads_.emplace_back([this, i] { io_services_[i].run(); });
    }
}

IOServicePool::~IOServicePool() {
    Stop();
    g_logger.info("IOServicePool Exit.");
}

IOServicePool::IOService& IOServicePool::GetIOService() {
    auto& io_service = io_services_[curr_idx_++];
    curr_idx_ %= io_services_.size();
    return io_service;
}

void IOServicePool::Stop() {
    // 当 io_service 已经绑定任务时, 需要先停止 io_service, 再释放 work
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

}  // namespace chat
}  // namespace pyc