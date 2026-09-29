module;

#include <memory>
#include <string_view>

#include "chat/server/proto/verify.pb.h"

export module chat.server.gate_server.verify_grpc_client;

import common.singleton;

export namespace pyc {
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
