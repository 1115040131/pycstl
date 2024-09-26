#pragma once

#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "common/singleton.h"

namespace pyc {
namespace chat {

class IOServicePool : public pyc::Singleton<IOServicePool> {
    friend class pyc::Singleton<IOServicePool>;

public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::io_context::work;
    using WorkPtr = std::unique_ptr<Work>;

    /// @brief 使用 round-robin 返回一个 io_context
    IOService& GetIOService();

    void Stop();

private:
    IOServicePool(std::size_t size = std::max(1u, std::thread::hardware_concurrency() / 4));

    ~IOServicePool();

private:
    std::vector<IOService> io_services_{};
    std::vector<WorkPtr> works_{};  // 防止 IOService run 后直接返回
    std::vector<std::thread> threads_{};
    std::size_t curr_idx_ = 0;  // 当前 io_service 下标
};

}  // namespace chat
}  // namespace pyc