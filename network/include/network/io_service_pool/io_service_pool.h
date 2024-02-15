#pragma once

#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "common/singleton.h"
#include "network/base/pool.h"

namespace network {

namespace asio = boost::asio;

class IOServicePool : public Pool, public pyc::Singleton<IOServicePool> {
    friend class pyc::Singleton<IOServicePool>;

public:
    /// @brief 使用 round-robin 返回一个 io_context
    IOService& GetIOService() override;

    void Stop() override;

private:
    IOServicePool(std::size_t size = std::max(1u, std::thread::hardware_concurrency() / 2));

    ~IOServicePool();

private:
    std::vector<IOService> io_services_;
    std::vector<std::unique_ptr<Work>> works_;  // 防止 IOService run 后直接返回
    std::vector<std::thread> threads_;
    std::size_t curr_idx_ = 0;  // 当前 io_service 下标
};

}  // namespace network
