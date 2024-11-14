#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "chat/server/common/data.h"
#include "common/singleton.h"

namespace pyc {
namespace chat {

class MysqlPool;  // 前置声明

class MysqlMgr : public Singleton<MysqlMgr> {
    friend class Singleton<MysqlMgr>;

private:
    MysqlMgr();

public:
    ~MysqlMgr();

    std::optional<int> RegUser(std::string_view name, std::string_view email, std::string_view password);

    std::optional<bool> CheckEmail(std::string_view name, std::string_view email);

    std::optional<bool> UpdatePassword(std::string_view name, std::string_view password);

    std::optional<UserInfo> CheckPassword(std::string_view email, std::string_view password);

    std::optional<UserInfo> GetUser(int uid);

private:
    std::unique_ptr<MysqlPool> pool_;
};

}  // namespace chat
}  // namespace pyc
