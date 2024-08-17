#include "gate_server/verify_grpc_client.h"

#include "gate_server/config_mgr.h"
#include "gate_server/define.h"

namespace pyc {
namespace chat {

RpcConnectionPool::RpcConnectionPool(std::size_t size, std::string_view host, std::string_view port) {
    auto target = fmt::format("{}:{}", host, port);
    for (std::size_t i = 0; i < size; ++i) {
        connections_.push(VerifyService::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials())));
    }
}

RpcConnectionPool::~RpcConnectionPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    Close();
    while (!connections_.empty()) {
        connections_.pop();
    }
}

void RpcConnectionPool::Close() {
    stop_ = true;
    cond_.notify_all();
}

std::unique_ptr<VerifyService::Stub> RpcConnectionPool::GetConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return !connections_.empty() || stop_; });

    if (stop_) {
        return nullptr;
    }

    auto connection = std::move(connections_.front());
    connections_.pop();
    return connection;
}

void RpcConnectionPool::ReturnConnection(std::unique_ptr<VerifyService::Stub> connection) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (stop_) {
        return;
    }

    connections_.push(std::move(connection));
    cond_.notify_one();
}

VerifyGrpcClient::VerifyGrpcClient() {
    GET_CONFIG(host, "VerifyServer", "Host");
    GET_CONFIG(port, "VerifyServer", "Port");
    pool_ = std::make_unique<RpcConnectionPool>(5, host, port);
}

VerifyResponse VerifyGrpcClient::GetVerifyCode(std::string_view email) {
    grpc::ClientContext context;
    VerifyRequest request;
    request.set_email(email);
    VerifyResponse response;

    auto stub = pool_->GetConnection();

    auto status = stub->GetVerifyCode(&context, request, &response);
    if (status.ok()) {
        pool_->ReturnConnection(std::move(stub));
        return response;
    }

    pool_->ReturnConnection(std::move(stub));
    response.set_error(static_cast<int32_t>(ErrorCode::kRpcFailed));
    return response;
}

}  // namespace chat
}  // namespace pyc
