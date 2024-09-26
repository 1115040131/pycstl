#include <boost/asio.hpp>

#include "chat/server/common/config_mgr.h"
#include "chat/server/status_server/define.h"
#include "chat/server/status_server/status_service_impl.h"

void RunServer() {
    GET_CONFIG(host, "StatusServer", "Host");
    GET_CONFIG(port, "StatusServer", "Port");

    auto server_address = host + ":" + port;
    pyc::chat::StatusServiceImpl service;

    // 监听端口和添加服务
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // 构建并启动服务
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    PYC_LOG_INFO("Server listening on {}", server_address);

    boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

    // 设置异步等待信号
    signals.async_wait([&](const boost::system::error_code& error, int) {
        if (!error) {
            server->Shutdown();
            io_context.stop();
        }
    });

    // 在单独的线程中运行 io_context
    std::thread([&io_context] { io_context.run(); }).detach();

    server->Wait();
}

int main() {
    try {
        RunServer();
    } catch (std::exception const& e) {
        PYC_LOG_FATAL("Error: {}", e.what());
        return EXIT_FAILURE;
    }
    return 0;
}