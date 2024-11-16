#include "chat/server/common/redis_mgr.h"

#include "chat/server/common/config_mgr.h"
#include "chat/server/common/redis_connection_pool.h"

namespace pyc {
namespace chat {

RedisMgr::RedisMgr() {
    GET_CONFIG(host, "Redis", "Host");
    GET_CONFIG_INT(port, "Redis", "Port");
    GET_CONFIG(password, "Redis", "Password");
    pool_ = std::make_unique<RedisConnectionPool>(5, host, port, password);
}

RedisMgr::~RedisMgr() { pool_->Close(); }

std::optional<std::string> RedisMgr::Get(std::string_view key) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return std::nullopt;
    }

    auto command = fmt::format("GET {}", key);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    _g_redis_mgr_logger.debug("[{}] success, value: {}", command, value);
    return value;
}

bool RedisMgr::Set(std::string_view key, std::string_view value) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto command = fmt::format("SET {} {}", key, value);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!RedisReplyOk(guard.reply_)) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return false;
    }

    _g_redis_mgr_logger.debug("[{}] success", command);
    return true;
}

bool RedisMgr::LPush(std::string_view key, std::string_view value) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto command = fmt::format("LPush {} {}", key, value);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    _g_redis_mgr_logger.debug("[{}] success, integer: {}", command, guard.reply_->integer);
    return true;
}

std::optional<std::string> RedisMgr::LPop(std::string_view key) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return std::nullopt;
    }

    auto command = fmt::format("LPop {}", key);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    _g_redis_mgr_logger.debug("[{}] success, value: {}", command, value);
    return value;
}

bool RedisMgr::RPush(std::string_view key, std::string_view value) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto command = fmt::format("RPush {} {}", key, value);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    _g_redis_mgr_logger.debug("[{}] success, integer: {}", command, guard.reply_->integer);
    return true;
}

std::optional<std::string> RedisMgr::RPop(std::string_view key) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return std::nullopt;
    }

    auto command = fmt::format("RPop {}", key);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    _g_redis_mgr_logger.debug("[{}] success, value: {}", command, value);
    return value;
}

bool RedisMgr::HSet(std::string_view key, std::string_view field, std::string_view value) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto command = fmt::format("HSET {} {} {}", key, field, value);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    _g_redis_mgr_logger.debug("[{}] success", command);
    return true;
}

std::optional<std::string> RedisMgr::HGet(std::string_view key, std::string_view field) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return std::nullopt;
    }

    auto command = fmt::format("HGET {} {}", key, field);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    _g_redis_mgr_logger.debug("[{}] success, value: {}", command, value);
    return value;
}

std::optional<int64_t> RedisMgr::HIncrBy(std::string_view key, std::string_view field, int64_t increment) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return std::nullopt;
    }

    auto command = fmt::format("HINCRBY {} {} {}", key, field, increment);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return std::nullopt;
    }

    auto value = guard.reply_->integer;
    _g_redis_mgr_logger.debug("[{}] success, value: {}", command, value);
    return value;
}

bool RedisMgr::Del(std::string_view key) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto command = fmt::format("DEL {}", key);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    bool del = guard.reply_->integer == 1;
    _g_redis_mgr_logger.debug("[{}] success, del: {}", command, del);
    return del;
}

bool RedisMgr::Exists(std::string_view key) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto command = fmt::format("EXISTS {}", key);
    auto guard = RedisReplyGuard(connection->Get(), command);

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[{}] failed", command);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[{}] failed, type: {}", command, guard.reply_->type);
        return false;
    }

    bool exists = guard.reply_->integer == 1;
    _g_redis_mgr_logger.debug("[{}] success, exists: {}", command, exists);
    return exists;
}

}  // namespace chat
}  // namespace pyc
