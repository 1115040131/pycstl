#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include "network/thread_pool_server.h"

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

        network::ThreadPoolServer server(io_context, 10086);
        server.StartAccept();
        {
            std::unique_lock lock(mutex_quit);

            cond_quit.wait(lock, [] { return is_stop.load(); });
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}