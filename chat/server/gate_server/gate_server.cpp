#include "chat/server/common/config_mgr.h"
#include "chat/server/gate_server/cserver.h"
#include "chat/server/gate_server/define.h"

int main() {
    GET_CONFIG_INT(port, "GateServer", "Port");

    try {
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context](const boost::system::error_code& ec, int) {
            if (ec) {
                return;
            }
            io_context.stop();
        });

        std::make_shared<pyc::chat::CServer>(io_context, port)->Start();
        PYC_LOG_INFO("Gate Server listening on port {}", port);
        io_context.run();
    } catch (const std::exception& e) {
        PYC_LOG_ERROR("{}", e.what());
    }

    return 0;
}