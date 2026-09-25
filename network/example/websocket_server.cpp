#include <boost/asio.hpp>

import network.websocket_server;

int main() {
    boost::asio::io_context io_context;
    network::WebsocketServer server(io_context, 10086);
    server.StartAccept();
    io_context.run();
    return 0;
}