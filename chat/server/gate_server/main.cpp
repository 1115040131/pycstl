#include <iostream>

#include <fmt/ostream.h>

#include "gate_server/cserver.h"

int main() {
    try {
        unsigned short port = 8080;
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context](const boost::system::error_code& ec, int) {
            if (ec) {
                return;
            }
            io_context.stop();
        });

        std::make_shared<pyc::chat::CServer>(io_context, port)->Start();
        io_context.run();
    } catch (const std::exception& e) {
        fmt::print(std::cerr, "{}\n", e.what());
    }

    return 0;
}