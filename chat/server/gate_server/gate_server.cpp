#include <exception>
#include <memory>

#include <boost/asio.hpp>

#include "logger/logger.h"

import chat.server.common.config_mgr;
import chat.server.gate_server;
import chat.server.gate_server.define;

int main() {
    auto port = pyc::chat::GetConfigIntOrDie("GateServer", "Port");

    try {
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context](const boost::system::error_code& ec, int) {
            if (ec) {
                return;
            }
            io_context.stop();
        });

        std::make_shared<pyc::chat::CServer>(static_cast<unsigned short>(port))->Start();
        pyc::chat::LogInfo("Gate Server listening on port {}", port);
        io_context.run();
    } catch (const std::exception& e) {
        pyc::chat::LogError("{}", e.what());
    }

    return 0;
}