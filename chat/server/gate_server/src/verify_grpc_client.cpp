#include "gate_server/verify_grpc_client.h"

#include <grpcpp/create_channel.h>

#include "chat/server/common/config_mgr.h"
#include "chat/server/common/defer.h"
#include "common/connection_pool.h"
#include "gate_server/define.h"

namespace pyc {
namespace chat {

class VerifyConnectionPool : public ConnectionPool<std::unique_ptr<VerifyService::Stub>> {
public:
    VerifyConnectionPool(std::string_view host, std::string_view port, size_t size) {
        auto target = fmt::format("{}:{}", host, port);
        for (std::size_t i = 0; i < size; ++i) {
            connections_.push(
                VerifyService::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials())));
        }
    }
};

VerifyGrpcClient::VerifyGrpcClient() {
    GET_CONFIG(host, "VerifyServer", "Host");
    GET_CONFIG(port, "VerifyServer", "Port");
    pool_ = std::make_unique<VerifyConnectionPool>(host, port, 5);
}

VerifyGrpcClient::~VerifyGrpcClient() { pool_->Close(); }

VerifyResponse VerifyGrpcClient::GetVerifyCode(std::string_view email) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        return {};
    }
    Defer defer([this, &connection]() { this->pool_->ReturnConnection(std::move(*connection)); });

    grpc::ClientContext context;
    VerifyRequest request;
    request.set_email(email);
    VerifyResponse response;

    auto status = connection.value()->GetVerifyCode(&context, request, &response);
    if (status.ok()) {
        return response;
    }

    response.set_error(static_cast<int32_t>(ErrorCode::kRpcFailed));
    return response;
}

}  // namespace chat
}  // namespace pyc
