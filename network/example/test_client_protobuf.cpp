#include <chrono>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>
#include <fmt/printf.h>

#include "network/msg_node.h"
#include "network/utils.h"
#include "proto/msg.pb.h"

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
                MsgData msg_data;
                msg_data.set_id(static_cast<decltype(msg_data.id())>(MsgId::kMsgHelloWorld));
                msg_data.set_data("Hello world!");
                std::string request = msg_data.SerializeAsString();
                SendNode send_data(request.data(), request.length(), MsgId::kMsgHelloWorld);
                asio::write(sock, asio::buffer(send_data.Data(), send_data.Size()));
            }
        });

        std::thread recv_thread([&sock]() {
            for (;;) {
                std::this_thread::sleep_for(2ms);
                char reply_head[kHeadLength];
                asio::read(sock, asio::buffer(reply_head, kHeadLength));
                MsgHead msg_head = MsgHead::ParseHead(reply_head);
                fmt::println("Replay head: {}", msg_head);
                char msg[kMaxLength];
                size_t msg_length = asio::read(sock, asio::buffer(msg, msg_head.length));
                MsgData recv_data;
                recv_data.ParseFromArray(msg, msg_length);
                fmt::println("Reply: {}\nReply len = {}", recv_data, msg_length);
            }
        });

        send_thread.join();
        recv_thread.join();

    } catch (const std::exception& e) {
        std::cerr << fmt::format("Exception: {}\n", e.what());
    }

    return 0;
}
