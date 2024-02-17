#include <fmt/core.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "network/proto/greeter.grpc.pb.h"

namespace network {

class GreeterServiceImpl final : public Greeter::Service {
    grpc::Status SayHello([[maybe_unused]] grpc::ServerContext* context, const network::HelloRequest* request,
                          network::HelloReply* response) override {
        response->set_message(fmt::format("Greeter server receive: \"{}\"", request->message()));
        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string server_address("127.0.0.1:50051");
    GreeterServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    fmt::println("Server listening on {}", server_address);
    server->Wait();
}

}  // namespace network

int main() {
    network::RunServer();
    return 0;
}