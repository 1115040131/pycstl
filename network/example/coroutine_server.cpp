#include <iostream>

#include "network/coroutine/coroutine_server.h"

int main() {
    try {
        auto& pool = network::IOServicePool::GetInstance();
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            io_context.stop();
            pool.Stop();
        });
        network::CoroutineServer server(io_context, 10086);
        server.StartAccept();
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}