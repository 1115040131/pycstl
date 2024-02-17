#include <memory>
#include <thread>

#include <fmt/core.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>

#include "common/timer.h"
#include "network/proto/greeter.grpc.pb.h"

using namespace std::literals::chrono_literals;

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
    pyc::Timer timer;

    constexpr std::size_t kConnectionNum = 100;  // 创建 100 个连接
    constexpr std::size_t kSendNum = 500;        // 每个连接循环发送 500 次

    std::vector<std::thread> threads;
    threads.reserve(kConnectionNum);

    for (std::size_t i = 0; i < kConnectionNum; ++i) {
        threads.emplace_back([] {
            auto channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
            network::GreeterClient client(channel);

            for (std::size_t j = 0; j < kSendNum; ++j) {
                auto result = client.SayHello("hello world");
                fmt::println("Greeter client receive: \"{}\"", result);
            }
        });

        std::this_thread::sleep_for(10ms);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
