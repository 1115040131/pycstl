#include <iostream>

#include "network/io_service_pool_server/server.h"

int main() {
    try {
        auto& pool = network::IOServicePool::Instance();
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            io_context.stop();
            pool.Stop();
        });
        network::IOServicePoolServer server(io_context, 10086);
        server.StartAccept();
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}