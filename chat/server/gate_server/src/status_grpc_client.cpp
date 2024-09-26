#include "gate_server/status_grpc_client.h"

#include <grpcpp/create_channel.h>

#include "chat/server/common/config_mgr.h"
#include "chat/server/common/defer.h"
#include "common/connection_pool.h"
#include "gate_server/define.h"

namespace pyc {
namespace chat {

class StatusConnectionPool : public ConnectionPool<std::unique_ptr<StatusService::Stub>> {
public:
    StatusConnectionPool(std::string_view host, std::string_view port, size_t size) {
        auto target = fmt::format("{}:{}", host, port);
        for (std::size_t i = 0; i < size; ++i) {
            connections_.push(
                StatusService::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials())));
        }
    }
};

StatusGrpcClient::StatusGrpcClient() {
    GET_CONFIG(host, "StatusServer", "Host");
    GET_CONFIG(port, "StatusServer", "Port");
    pool_ = std::make_unique<StatusConnectionPool>(host, port, 5);
}

StatusGrpcClient::~StatusGrpcClient() { pool_->Close(); }

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        return {};
    }
    Defer defer([this, &connection]() { this->pool_->ReturnConnection(std::move(*connection)); });

    grpc::ClientContext context;
    GetChatServerReq request;
    request.set_uid(uid);

    GetChatServerRsp response;
    auto status = connection.value()->GetChatServer(&context, request, &response);
    if (status.ok()) {
        return response;
    }

    response.set_error(static_cast<int32_t>(ErrorCode::kRpcFailed));
    return response;
}

}  // namespace chat
}  // namespace pyc