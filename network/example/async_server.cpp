#include <iostream>

#include <boost/asio.hpp>

#include "network/server.h"

int main() {
    try {
        boost::asio::io_context io_context;
        network::Server server(io_context, 10086);
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}