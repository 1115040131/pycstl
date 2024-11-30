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

    std::optional<UserInfo> GetUser(std::string_view name);

    std::optional<bool> AddFriendAppply(int from_uid, int to_uid);

    std::optional<std::vector<ApplyInfo>> GetApplyList(int to_uid, int begin_index, int limit = 10);

    std::optional<bool> AuthFriendApply(int from_uid, int to_uid);

    std::optional<bool> AddFriend(int from_uid, int to_uid, std::string_view back_name);

    std::optional<std::vector<UserInfo>> GetFriendList(int uid);

private:
    std::unique_ptr<MysqlPool> pool_;
};

}  // namespace chat
}  // namespace pyc
