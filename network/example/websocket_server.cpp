#include "network/websocket/websocket_server.h"

int main() {
    boost::asio::io_context io_context;
    network::WebsocketServer server(io_context, 10086);
    server.StartAccept();
    io_context.run();
    return 0;
}