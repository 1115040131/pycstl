#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "common/singleton.h"

namespace pyc {
namespace chat {

// redis key 定义
constexpr inline std::string_view kLoginCount = "login_count";
constexpr inline std::string_view kUserTokenPrefix = "utoken_";

class RedisConnectionPool;

class RedisMgr : public Singleton<RedisMgr> {
    friend class Singleton<RedisMgr>;

private:
    RedisMgr();

public:
    ~RedisMgr();

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
    std::unique_ptr<RedisConnectionPool> pool_;
};

}  // namespace chat
}  // namespace pyc
