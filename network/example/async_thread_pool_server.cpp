#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include "network/thread_pool/thread_pool_server.h"


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