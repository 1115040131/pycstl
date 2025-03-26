#pragma once

#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "common/singleton.h"
#include "network/base/pool.h"

namespace network {

namespace asio = boost::asio;

class ThreadPool : public Pool, public pyc::Singleton<ThreadPool> {
    friend class pyc::Singleton<ThreadPool>;

public:
    IOService& GetIOService() override;

    void Stop() override;

private:
    ThreadPool(std::size_t size = std::max(1u, std::thread::hardware_concurrency() / 2));

    ~ThreadPool();

private:
    IOService io_service_{};
    Work work_ = asio::make_work_guard(io_service_);
    std::vector<std::thread> threads_{};
};

}  // namespace network
