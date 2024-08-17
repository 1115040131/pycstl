#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string_view>

#include <grpcpp/create_channel.h>

#include "chat/server/proto/verify.grpc.pb.h"
#include "common/singleton.h"

namespace pyc {
namespace chat {

class RpcConnectionPool {
public:
    RpcConnectionPool(std::size_t size, std::string_view host, std::string_view port);

    ~RpcConnectionPool();

    void Close();

    std::unique_ptr<VerifyService::Stub> GetConnection();

    void ReturnConnection(std::unique_ptr<VerifyService::Stub> connection);

private:
    std::atomic<bool> stop_{false};
    std::queue<std::unique_ptr<VerifyService::Stub>> connections_;
    std::condition_variable cond_;
    std::mutex mutex_;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient> {
    friend class Singleton<VerifyGrpcClient>;

public:
    VerifyResponse GetVerifyCode(std::string_view email);

private:
    VerifyGrpcClient();

private:
    std::unique_ptr<RpcConnectionPool> pool_;
};

}  // namespace chat
}  // namespace pyc
