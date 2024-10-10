#pragma once

#include "chat/server/proto/status.grpc.pb.h"
#include "common/singleton.h"

namespace pyc {
namespace chat {

class StatusConnectionPool;

class StatusGrpcClient : public Singleton<StatusGrpcClient> {
    friend class Singleton<StatusGrpcClient>;

private:
    StatusGrpcClient();

public:
    ~StatusGrpcClient();

    GetChatServerRsp GetChatServer(int uid);

    LoginRsp Login(int uid, const std::string& token);

private:
    std::unique_ptr<StatusConnectionPool> pool_;
};

}  // namespace chat
}  // namespace pyc
