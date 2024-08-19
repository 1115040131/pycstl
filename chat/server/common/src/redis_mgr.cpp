#include "chat/server/common/redis_mgr.h"

#include <fmt/format.h>

namespace pyc {
namespace chat {

RedisMgr::RedisMgr() {}

RedisMgr::~RedisMgr() { Close(); }

void RedisMgr::Close() {
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }
}

bool RedisReplyOk(redisReply* reply) {
    return reply && reply->type == REDIS_REPLY_STATUS && ::strcasecmp(reply->str, "OK") == 0;
}

bool RedisMgr::Connect(std::string_view host, int port) {
    context_ = redisConnect(host.data(), port);
    if (!context_ || context_->err) {
        fmt::println("Connect error: {}", context_->errstr);
        return false;
    }
    return true;
}

bool RedisMgr::Auth(std::string_view password) {
    auto command = fmt::format("AUTH {}", password);
    auto guard = ReplyGuard(context_, command);

    if (!RedisReplyOk(guard.reply_)) {
        fmt::println("[{}] failed", command);
        return false;
    }

    fmt::println("[{}] success", command);
    return true;
}

std::optional<std::string> RedisMgr::Get(std::string_view key) {
    auto command = fmt::format("GET {}", key);
    auto guard = ReplyGuard(context_, command);

    if (!guard.reply_) {
        fmt::println("[{}] failed", command);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        fmt::println("[{}] failed, type: {}", command, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    fmt::println("[{}] success, value: {}", command, value);
    return value;
}

bool RedisMgr::Set(std::string_view key, std::string_view value) {
    auto command = fmt::format("SET {} {}", key, value);
    auto guard = ReplyGuard(context_, command);

    if (!RedisReplyOk(guard.reply_)) {
        fmt::println("[{}] failed", command);
        return false;
    }

    fmt::println("[{}] success", command);
    return true;
}

bool RedisMgr::LPush(std::string_view key, std::string_view value) {
    auto command = fmt::format("LPush {} {}", key, value);
    auto guard = ReplyGuard(context_, command);

    if (!guard.reply_) {
        fmt::println("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        fmt::println("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    fmt::println("[{}] success, integer: {}", command, guard.reply_->integer);
    return true;
}

std::optional<std::string> RedisMgr::LPop(std::string_view key) {
    auto command = fmt::format("LPop {}", key);
    auto guard = ReplyGuard(context_, command);

    if (!guard.reply_) {
        fmt::println("[{}] failed", command);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        fmt::println("[{}] failed, type: {}", command, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    fmt::println("[{}] success, value: {}", command, value);
    return value;
}

bool RedisMgr::RPush(std::string_view key, std::string_view value) {
    auto command = fmt::format("RPush {} {}", key, value);
    auto guard = ReplyGuard(context_, command);

    if (!guard.reply_) {
        fmt::println("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        fmt::println("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    fmt::println("[{}] success, integer: {}", command, guard.reply_->integer);
    return true;
}

std::optional<std::string> RedisMgr::RPop(std::string_view key) {
    auto command = fmt::format("RPop {}", key);
    auto guard = ReplyGuard(context_, command);

    if (!guard.reply_) {
        fmt::println("[{}] failed", command);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        fmt::println("[{}] failed, type: {}", command, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    fmt::println("[{}] success, value: {}", command, value);
    return value;
}

bool RedisMgr::HSet(std::string_view key, std::string_view field, std::string_view value) {
    auto command = fmt::format("HSET {} {} {}", key, field, value);
    auto guard = ReplyGuard(context_, command);

    if (!guard.reply_) {
        fmt::println("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        fmt::println("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    fmt::println("[{}] success", command);
    return true;
}

std::optional<std::string> RedisMgr::HGet(std::string_view key, std::string_view field) {
    auto command = fmt::format("HGET {} {}", key, field);
    auto guard = ReplyGuard(context_, command);

    if (!guard.reply_) {
        fmt::println("[{}] failed", command);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        fmt::println("[{}] failed, type: {}", command, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    fmt::println("[{}] success, value: {}", command, value);
    return value;
}

bool RedisMgr::Del(std::string_view key) {
    auto command = fmt::format("DEL {}", key);
    auto guard = ReplyGuard(context_, command);

    if (!guard.reply_) {
        fmt::println("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        fmt::println("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    bool del = guard.reply_->integer == 1;
    fmt::println("[{}] success, del: {}", command, del);
    return del;
}

bool RedisMgr::Exists(std::string_view key) {
    auto command = fmt::format("EXISTS {}", key);
    auto guard = ReplyGuard(context_, command);

    if (!guard.reply_) {
        fmt::println("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        fmt::println("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    bool exists = guard.reply_->integer == 1;
    fmt::println("[{}] success, exists: {}", command, exists);
    return exists;
}

}  // namespace chat
}  // namespace pyc
