#pragma once

#include <boost/asio.hpp>

namespace network {

namespace asio = boost::asio;

class Pool {
public:
    using IOService = asio::io_context;
    using Work = asio::executor_work_guard<asio::io_context::executor_type>;

    virtual ~Pool() = default;

    virtual IOService& GetIOService() = 0;

    virtual void Stop() = 0;
};

}  // namespace network
