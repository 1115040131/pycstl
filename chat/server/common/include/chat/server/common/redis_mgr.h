#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <hiredis.h>

#include "common/singleton.h"

namespace pyc {
namespace chat {

class RedisMgr : public Singleton<RedisMgr> {
    friend class Singleton<RedisMgr>;

public:
    ~RedisMgr();

    void Close();

    bool Connect(std::string_view host, int port);
    bool Auth(std::string_view password);

    std::optional<std::string> Get(std::string_view key);
    bool Set(std::string_view key, std::string_view value);

    bool LPush(std::string_view key, std::string_view value);
    std::optional<std::string> LPop(std::string_view key);
    bool RPush(std::string_view key, std::string_view value);
    std::optional<std::string> RPop(std::string_view key);

    bool HSet(std::string_view key, std::string_view field, std::string_view value);
    std::optional<std::string> HGet(std::string_view key, std::string_view field);

    bool Del(std::string_view key);

    bool Exists(std::string_view key);

private:
    RedisMgr();

    struct ReplyGuard {
        explicit ReplyGuard(redisReply* reply) : reply_(reply) {}

        ReplyGuard(redisContext* context, std::string_view command)
            : reply_(static_cast<redisReply*>(redisCommand(context, command.data()))) {}

        ~ReplyGuard() { freeReplyObject(reply_); }

        redisReply* reply_;
    };

private:
    redisContext* context_;
    // redisReply* reply_;
};

}  // namespace chat
}  // namespace pyc
