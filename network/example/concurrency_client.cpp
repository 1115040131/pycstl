#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "common/timer.h"
#include "network/msg_node.h"
#include "network/utils.h"

namespace asio = boost::asio;

using namespace network;
using namespace std::literals::chrono_literals;

int main() {
    Timer timer;

    constexpr size_t kConnectionNum = 100;  // 创建 100 个连接
    constexpr size_t kSendNum = 500;        // 每个连接循环发送 500 次

    std::vector<std::thread> threads;
    threads.reserve(kConnectionNum);

    for (size_t i = 0; i < kConnectionNum; ++i) {
        threads.emplace_back([] {
            try {
                // 创建上下文
                asio::io_context io_context;

                // 构造 endpoint
                asio::ip::tcp::endpoint remote_endpoint(asio::ip::address::from_string("127.0.0.1"), 10086);
                asio::ip::tcp::socket sock(io_context);
                boost::system::error_code error_code = boost::asio::error::host_not_found;
                sock.connect(remote_endpoint, error_code);
                if (error_code) {
                    fmt::println("connect failed! Error code = {}. Message: {}", error_code.value(),
                                 error_code.message());
                    return;
                }

                for (size_t j = 0; j < kSendNum; ++j) {
                    nlohmann::json root;
                    root["id"] = MsgId::kMsgHelloWorld;
                    root["data"] = "Hello world!";
                    std::string request = root.dump();
                    SendNode send_data(request.data(), request.length(), MsgId::kMsgHelloWorld);
                    asio::write(sock, asio::buffer(send_data.Data(), send_data.Size()));

                    char reply_head[kHeadLength];
                    asio::read(sock, asio::buffer(reply_head, kHeadLength));
                    MsgHead msg_head = MsgHead::ParseHead(reply_head);
                    fmt::println("Replay head: {}", msg_head);
                    char msg[kMaxLength];
                    size_t msg_length = asio::read(sock, asio::buffer(msg, msg_head.length));
                    auto reader = nlohmann::json::parse(msg, msg + msg_length);
                    fmt::println("Reply msg id = {}, data = \"{}\"", reader["id"].get<MsgId>(),
                                 reader["data"].get<std::string>());
                }

            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n';
            }
        });

        std::this_thread::sleep_for(10ms);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
