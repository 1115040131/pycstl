#include <cstdlib>
#include <exception>
#include <thread>

#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>

#include "logger/logger.h"

import chat.server.common.config_mgr;
import chat.server.status_server.define;
import chat.server.status_server.status_service_impl;

void RunServer() {
    auto host = pyc::chat::GetConfigOrDie("StatusServer", "Host");
    auto port = pyc::chat::GetConfigOrDie("StatusServer", "Port");

    auto server_address = host + ":" + port;
    pyc::chat::StatusServiceImpl service;

    // 监听端口和添加服务
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // 构建并启动服务
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    pyc::chat::LogInfo("Server listening on {}", server_address);

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
        pyc::chat::LogFatal("Error: {}", e.what());
        return EXIT_FAILURE;
    }
    return 0;
}