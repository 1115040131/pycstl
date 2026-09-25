#include <exception>
#include <memory>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>

#include "common/utils.h"
#include "logger/logger.h"

import chat.server.chat_server;
import chat.server.chat_server.chat_service_impl;
import chat.server.common.config_mgr;
import chat.server.common.redis_mgr;

using namespace pyc::chat;

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        LogFatal("Usage: {} <chat_server_name>", argv[0]);
        return 1;
    }

    try {
        std::string chat_server_name = argv[1];

        SetSectionOrDie(chat_server_name);

        // 将登录数设置为0
        RedisMgr::GetInstance().HSet(kLoginCount, chat_server_name, "0");

        // 定义一个 GrpcServer
        auto host = GetSectionConfigOrDie("Host");
        auto rpc_port = GetSectionConfigOrDie("RpcPort");
        auto server_address = pyc::JoinHostPort(host, rpc_port);
        ChatServiceImpl service;
        // 监听端口和添加服务
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        // 构建并启动服务
        std::unique_ptr<grpc::Server> grpc_server(builder.BuildAndStart());
        // 单独起一个线程处理 grpc 服务
        std::jthread grpc_server_thread([&grpc_server, &server_address]() {
            LogInfo("Server listening on {}", server_address);
            grpc_server->Wait();
        });

        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context, &grpc_server](const boost::system::error_code& ec, int) {
            if (ec) {
                return;
            }
            io_context.stop();
            grpc_server->Shutdown();
        });

        auto port = GetSectionConfigIntOrDie("Port");
        CServer server(io_context, chat_server_name, port);
        io_context.run();

        RedisMgr::GetInstance().HDel(kLoginCount, chat_server_name);
    } catch (const std::exception& e) {
        LogError("{}", e.what());
    }

    return 0;
}