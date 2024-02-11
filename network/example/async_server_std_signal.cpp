#include <atomic>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>

#include "network/server.h"

static std::atomic<bool> is_stop{false};
static std::condition_variable cond_quit;

void SigHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        is_stop = true;
        cond_quit.notify_one();
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        std::thread network_thread([&io_context] {
            network::Server server(io_context, 10086);
            io_context.run();
        });

        std::signal(SIGINT, SigHandler);
        std::signal(SIGTERM, SigHandler);

        std::mutex mutex_quit;
        std::unique_lock<std::mutex> lock(mutex_quit);
        cond_quit.wait(lock, [] { return is_stop.load(); });

        io_context.stop();
        network_thread.join();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}