#include <memory>

#include <fmt/core.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include "network/proto/greeter.grpc.pb.h"
#pragma GCC diagnostic pop

namespace network {

class GreeterClient {
public:
    GreeterClient(const std::shared_ptr<grpc::Channel>& channel) : stub_(Greeter::NewStub(channel)) {}

    std::string SayHello(const std::string& user) {
        HelloRequest request;
        request.set_message(user);

        HelloReply reply;
        grpc::ClientContext context;

        grpc::Status status = stub_->SayHello(&context, request, &reply);
        if (status.ok()) {
            return reply.message();
        } else {
            return fmt::format("[{}]: error_message: {}", __func__, status.error_message());
        }
    }

private:
    std::unique_ptr<Greeter::Stub> stub_;
};

}  // namespace network

int main() {
    auto channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
    network::GreeterClient client(channel);
    auto result = client.SayHello("hello world");
    fmt::println("Greeter client receive: \"{}\"", result);

    return 0;
}
