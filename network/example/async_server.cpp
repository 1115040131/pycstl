#include <iostream>

#include <boost/asio.hpp>

#include "network/io_service_pool.h"
#include "network/server.h"

int main() {
    try {
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context](auto, auto) {
            io_context.stop();
            network::IOServicePool::Instance().Stop();
        });

        network::Server server(io_context, 10086);
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}