#pragma once

#include "chat/server/proto/verify.grpc.pb.h"
#include "common/singleton.h"

namespace pyc {
namespace chat {

class VerifyConnectionPool;

class VerifyGrpcClient : public Singleton<VerifyGrpcClient> {
    friend class Singleton<VerifyGrpcClient>;

private:
    VerifyGrpcClient();

public:
    ~VerifyGrpcClient();

    VerifyResponse GetVerifyCode(std::string_view email);

private:
    std::unique_ptr<VerifyConnectionPool> pool_;
};

}  // namespace chat
}  // namespace pyc
