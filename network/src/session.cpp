#include "network/session.h"

#include <fmt/printf.h>

namespace network {

void Session::Start() {
    memset(data_, 0, kMaxLength);
    socket_.async_read_some(asio::buffer(data_, kMaxLength),
                            [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                HandleRead(error_code, bytes_transferred);
                            });
}

void Session::HandleRead(const boost::system::error_code& error_code, size_t bytes_transferred) {
    if (!error_code) {
        fmt::println("[{}]: Server receive data {}", __func__, data_);
        asio::async_write(socket_, asio::buffer(data_, bytes_transferred),
                          [this](const boost::system::error_code& error_code,
                                 [[maybe_unused]] std::size_t bytes_transferred) { HandleWrite(error_code); });
    } else {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());

        // TODO: fix this
        delete this;
    }
}

void Session::HandleWrite(const boost::system::error_code& error_code) {
    if (!error_code) {
        memset(data_, 0, kMaxLength);
        socket_.async_read_some(asio::buffer(data_, kMaxLength),
                                [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                    HandleRead(error_code, bytes_transferred);
                                });
    } else {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());

        // TODO: fix this
        delete this;
    }
}

}  // namespace network
