#include <atomic>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>

#include <boost/asio.hpp>

import network.thread_pool;
import network.thread_pool_server;

int main() {
    try {
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context](auto, auto) {
            io_context.stop();
            network::ThreadPool::GetInstance().Stop();
        });

        network::ThreadPoolServer server(io_context, 10086);
        server.StartAccept();

        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}