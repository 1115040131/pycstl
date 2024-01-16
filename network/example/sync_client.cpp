#include <iostream>

#include <boost/asio.hpp>
#include <fmt/printf.h>

namespace asio = boost::asio;

static constexpr size_t kMaxLength = 1024;

int main() {
    try {
        // 创建上下文
        asio::io_context io_context;

        // 构造 endpoint
        asio::ip::tcp::endpoint remote_endpoint(asio::ip::address::from_string("127.0.0.1"), 10086);
        asio::ip::tcp::socket sock(io_context);
        boost::system::error_code error_code = boost::asio::error::host_not_found;
        sock.connect(remote_endpoint, error_code);
        if (error_code) {
            fmt::println("connect failed! Error code = {}. Message: {}", error_code.value(), error_code.message());
            return 0;
        }

        fmt::print("Enter message: ");
        char request[kMaxLength];
        std::cin.getline(request, kMaxLength);
        size_t request_length = strlen(request);
        asio::write(sock, asio::buffer(request, request_length));

        char reply[kMaxLength];
        size_t reply_length = asio::read(sock, asio::buffer(reply, request_length));
        fmt::println("Reply is: {:.{}}", reply, reply_length);
    } catch (const std::exception& e) {
        std::cerr << fmt::format("Exception: {}\n", e.what());
    }

    return 0;
}
