#include <iostream>
#include <memory>
#include <thread>
#include <unordered_set>

#include <boost/asio.hpp>
#include <fmt/printf.h>

namespace asio = boost::asio;

static constexpr size_t kMaxLength = 1024;

static std::unordered_set<std::shared_ptr<std::thread>> g_threads;

/// @brief 处理客户端连接后的数据
void Session(const std::shared_ptr<asio::ip::tcp::socket>& sock) {
    try {
        while (true) {
            char data[kMaxLength] = {0};
            boost::system::error_code error_code;
            size_t length = sock->read_some(asio::buffer(data, kMaxLength), error_code);
            if (error_code == asio::error::eof) {
                fmt::println("Connection closed by peer");
                break;
            } else if (error_code) {
                throw boost::system::system_error(error_code);
            }

            fmt::println("Receive from {}", sock->remote_endpoint().address().to_string());
            fmt::println("Receive message {:.{}}", data, length);

            // 回传
            asio::write(*sock, asio::buffer(data, length));
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

/// @brief 处理客户端连接
void Server(asio::io_context& io_context, unsigned short port_num) {
    asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_num));
    while (true) {
        auto sock = std::make_shared<asio::ip::tcp::socket>(io_context);
        acceptor.accept(*sock);
        auto thread = std::make_shared<std::thread>(Session, sock);
        g_threads.insert(thread);
    }
}

int main() {
    try {
        asio::io_context io_context;
        Server(io_context, 10086);
        for (auto& thread : g_threads) {
            thread->join();
        }
    } catch (const std::exception& e) {
        std::cerr << fmt::format("Exception: {}\n", e.what());
    }

    return 0;
}
