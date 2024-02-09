#include <chrono>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>
#include <fmt/printf.h>

#include "network/msg_node.h"

namespace asio = boost::asio;

using namespace std::literals;
using namespace network;

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

        std::thread send_thread([&sock]() {
            for (;;) {
                std::this_thread::sleep_for(2ms);
                const char* request = "Hello world!";
                MsgSizeType request_length = strlen(request);
                char send_data[kMaxLength] = {0};
                MsgSizeType request_length_network =
                    asio::detail::socket_ops::host_to_network_short(request_length);
                memcpy(send_data, &request_length_network, kHeadLength);
                memcpy(send_data + kHeadLength, request, request_length);
                asio::write(sock, asio::buffer(send_data, kHeadLength + request_length));
            }
        });

        std::thread recv_thread([&sock]() {
            for (;;) {
                std::this_thread::sleep_for(2ms);
                char reply_head[kHeadLength];
                asio::read(sock, asio::buffer(reply_head, kHeadLength));
                MsgSizeType msg_len;
                memcpy(&msg_len, reply_head, kHeadLength);
                msg_len = asio::detail::socket_ops::network_to_host_short(msg_len);
                char msg[kMaxLength];
                size_t msg_length = asio::read(sock, asio::buffer(msg, msg_len));
                fmt::println("Reply is: {:.{}}\nReply len is {}", msg, msg_length, msg_length);
            }
        });

        send_thread.join();
        recv_thread.join();

    } catch (const std::exception& e) {
        std::cerr << fmt::format("Exception: {}\n", e.what());
    }

    return 0;
}
