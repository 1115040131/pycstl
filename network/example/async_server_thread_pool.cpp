#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include <boost/asio.hpp>

#include "network/server.h"
#include "network/thread_pool.h"
#include <fmt/printf.h>

static std::atomic<bool> is_stop{false};
static std::condition_variable cond_quit;
static std::mutex mutex_quit;

int main() {
    try {
        boost::asio::io_context& io_context = network::ThreadPool::Instance().GetIOService();
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context](auto, auto) {
            io_context.stop();
            network::ThreadPool::Instance().Stop();
            is_stop = true;
            cond_quit.notify_one();
        });

        network::Server server(io_context, 10086);
        {
            std::unique_lock lock(mutex_quit);

            cond_quit.wait(lock, [] { return is_stop.load(); });
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}