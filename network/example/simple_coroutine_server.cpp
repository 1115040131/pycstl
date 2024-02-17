#include <iostream>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>

namespace asio = boost::asio;

using asio::ip::tcp;

static constexpr std::size_t kMaxSize = 1024;

asio::awaitable<void> Echo(tcp::socket socket) {
    try {
        char data[1024];
        for (;;) {
            std::size_t read_size =
                co_await socket.async_read_some(asio::buffer(data, kMaxSize), asio::use_awaitable);
            co_await asio::async_write(socket, asio::buffer(data, read_size), asio::use_awaitable);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

asio::awaitable<void> Listener() {
    // 通过 co_await 异步查询调度器,
    auto executor = co_await asio::this_coro::executor;  // 协程调度器
    tcp::acceptor acceptor(executor, tcp::endpoint(tcp::v4(), 10086));
    for (;;) {
        // use_awaitable 将异步函数声明为可等待的
        tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        asio::co_spawn(executor, Echo(std::move(socket)), asio::detached);
    }
}

int main() {
    try {
        asio::io_context io_context(1);
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });
        asio::co_spawn(io_context, Listener(), asio::detached);  // 启动一个协程, 独立执行
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}