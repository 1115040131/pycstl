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

    auto guard = RedisReplyGuard(connection->Get(), {"GET", key});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[GET {}] failed", key);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        _g_redis_mgr_logger.error("[GET {}] failed, type: {}", key, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    _g_redis_mgr_logger.debug("[GET {}] success, value: {}", key, value);
    return value;
}

bool RedisMgr::Set(std::string_view key, std::string_view value) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"SET", key, value});

    if (!RedisReplyOk(guard.reply_)) {
        _g_redis_mgr_logger.error("[SET {} {}] failed", key, value);
        return false;
    }

    _g_redis_mgr_logger.debug("[SET {} {}] success", key, value);
    return true;
}

bool RedisMgr::LPush(std::string_view key, std::string_view value) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"LPush", key, value});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[LPush {} {}] failed", key, value);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[LPush {} {}] failed, type: {}", key, value, guard.reply_->type);
        return false;
    }

    _g_redis_mgr_logger.debug("[LPush {} {}] success, integer: {}", key, value, guard.reply_->integer);
    return true;
}

std::optional<std::string> RedisMgr::LPop(std::string_view key) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return std::nullopt;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"LPop", key});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[LPop {}] failed", key);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        _g_redis_mgr_logger.error("[LPop {}] failed, type: {}", key, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    _g_redis_mgr_logger.debug("[LPop {}] success, value: {}", key, value);
    return value;
}

bool RedisMgr::RPush(std::string_view key, std::string_view value) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"RPush", key, value});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[RPush {} {}] failed", key, value);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[RPush {} {}] failed, type: {}", key, value, guard.reply_->type);
        return false;
    }

    _g_redis_mgr_logger.debug("[RPush {} {}] success, integer: {}", key, value, guard.reply_->integer);
    return true;
}

std::optional<std::string> RedisMgr::RPop(std::string_view key) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return std::nullopt;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"RPop", key});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[RPop {}] failed", key);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        _g_redis_mgr_logger.error("[RPop {}] failed, type: {}", key, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    _g_redis_mgr_logger.debug("[RPop {}] success, value: {}", key, value);
    return value;
}

bool RedisMgr::HDel(std::string_view key, std::string_view field) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"HDEL", key, field});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[HDEL {} {}] failed", key, field);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[HDEL {} {}] failed, type: {}", key, field, guard.reply_->type);
        return false;
    }

    bool del = guard.reply_->integer == 1;
    _g_redis_mgr_logger.debug("[HDEL {} {}] success, del: {}", key, field, del);
    return del;
}

bool RedisMgr::HSet(std::string_view key, std::string_view field, std::string_view value) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"HSET", key, field, value});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[HSET {} {} {}] failed", key, field, value);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[HSET {} {} {}] failed, type: {}", key, field, value, guard.reply_->type);
        return false;
    }

    _g_redis_mgr_logger.debug("[HSET {} {} {}] success", key, field, value);
    return true;
}

std::optional<std::string> RedisMgr::HGet(std::string_view key, std::string_view field) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return std::nullopt;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"HGET", key, field});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[HGET {} {}] failed", key, field);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_STRING) {
        _g_redis_mgr_logger.error("[HGET {} {}] failed, type: {}", key, field, guard.reply_->type);
        return std::nullopt;
    }

    std::string value = guard.reply_->str;
    _g_redis_mgr_logger.debug("[HGET {} {}] success, value: {}", key, field, value);
    return value;
}

std::optional<int64_t> RedisMgr::HIncrBy(std::string_view key, std::string_view field, int64_t increment) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return std::nullopt;
    }

    auto increment_str = std::to_string(increment);
    auto guard = RedisReplyGuard(connection->Get(), {"HINCRBY", key, field, increment_str});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[HINCRBY {} {} {}] failed", key, field, increment);
        return std::nullopt;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[HINCRBY {} {} {}] failed, type: {}", key, field, increment,
                                  guard.reply_->type);
        return std::nullopt;
    }

    auto value = guard.reply_->integer;
    _g_redis_mgr_logger.debug("[HINCRBY {} {} {}] success, value: {}", key, field, increment, value);
    return value;
}

bool RedisMgr::Del(std::string_view key) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"DEL", key});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[DEL {}] failed", key);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[DEL {}] failed, type: {}", key, guard.reply_->type);
        return false;
    }

    bool del = guard.reply_->integer == 1;
    _g_redis_mgr_logger.debug("[DEL {}] success, del: {}", key, del);
    return del;
}

bool RedisMgr::Exists(std::string_view key) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        _g_redis_mgr_logger.error("Get connection failed");
        return false;
    }

    auto guard = RedisReplyGuard(connection->Get(), {"EXISTS", key});

    if (!guard.reply_) {
        _g_redis_mgr_logger.error("[EXISTS {}] failed", key);
        return false;
    }
    if (guard.reply_->type != REDIS_REPLY_INTEGER) {
        _g_redis_mgr_logger.error("[EXISTS {}] failed, type: {}", key, guard.reply_->type);
        return false;
    }

    bool exists = guard.reply_->integer == 1;
    _g_redis_mgr_logger.debug("[EXISTS {}] success, exists: {}", key, exists);
    return exists;
}

}  // namespace chat
}  // namespace pyc
