#include "gate_server/config_mgr.h"
#include "gate_server/cserver.h"
#include "gate_server/define.h"

int main() {
    GET_CONFIG(port_str, "GateServer", "Port");

    char* end;
    unsigned short port = std::strtoul(port_str.c_str(), &end, 10);
    if (end == port_str.c_str() || *end != '\0') {
        PYC_LOG_ERROR("Config Port: {} Invalid", port_str);
        return 1;
    }

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
        PYC_LOG_INFO("Gate Server listening at port {}", port);
        io_context.run();
    } catch (const std::exception& e) {
        PYC_LOG_ERROR("{}", e.what());
    }

    return 0;
}