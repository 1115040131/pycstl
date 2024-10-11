#include "chat/server/chat_server/cserver.h"
#include "chat/server/chat_server/define.h"
#include "chat/server/common/config_mgr.h"

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        PYC_LOG_FATAL("Usage: {} <chat_server_name>", argv[0]);
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

        std::string chat_server_name = argv[1];
        GET_CONFIG_INT(port, chat_server_name, "Port");
        pyc::chat::CServer server(io_context, chat_server_name, port);
        io_context.run();
    } catch (const std::exception& e) {
        PYC_LOG_ERROR("{}", e.what());
    }

    return 0;
}