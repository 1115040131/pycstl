#include "gate_server/verify_grpc_client.h"

#include "gate_server/define.h"

namespace pyc {
namespace chat {

VerifyResponse VerifyGrpcClient::GetVerifyCode(std::string_view email) {
    grpc::ClientContext context;
    VerifyRequest request;
    request.set_email(email);
    VerifyResponse response;

    auto status = stub_->GetVerifyCode(&context, request, &response);
    if (status.ok()) {
        return response;
    }

    response.set_error(static_cast<int32_t>(ErrorCode::kRpcFailed));
    return response;
}

}  // namespace chat
}  // namespace pyc
