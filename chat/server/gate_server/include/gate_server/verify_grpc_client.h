#pragma once

#include <memory>
#include <string_view>

#include <grpcpp/create_channel.h>

#include "chat/server/gate_server/proto/verify.grpc.pb.h"
#include "common/singleton.h"

namespace pyc {
namespace chat {

class VerifyGrpcClient : public Singleton<VerifyGrpcClient> {
    friend class Singleton<VerifyGrpcClient>;

public:
    VerifyResponse GetVerifyCode(std::string_view email);

private:
    VerifyGrpcClient()
        : stub_(VerifyService::NewStub(grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials()))) {
    }

private:
    std::unique_ptr<VerifyService::Stub> stub_;
};

}  // namespace chat
}  // namespace pyc
