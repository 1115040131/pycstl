#include "chat/server/chat_server/cserver.h"
#include "chat/server/chat_server/define.h"
#include "chat/server/common/config_mgr.h"
#include "chat/server/common/redis_mgr.h"

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        PYC_LOG_FATAL("Usage: {} <chat_server_name>", argv[0]);
        return 1;
    }

    try {
        std::string chat_server_name = argv[1];

        // 将登录数设置为0
        pyc::chat::RedisMgr::GetInstance().HSet(pyc::chat::kLoginCount, chat_server_name, "0");

        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context](const boost::system::error_code& ec, int) {
            if (ec) {
                return;
            }
            io_context.stop();
        });

        SET_SECTION(chat_server_name);
        GET_SECTION_CONFIG_INT(port, "Port");
        pyc::chat::CServer server(io_context, chat_server_name, port);
        io_context.run();
    } catch (const std::exception& e) {
        PYC_LOG_ERROR("{}", e.what());
    }

    return 0;
}