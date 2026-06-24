#pragma once

#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "chat/server/chat_server/csession.h"
#include "common/singleton.h"

namespace pyc {
namespace chat {

class UserMgr : public Singleton<UserMgr> {
    friend class Singleton<UserMgr>;

private:
    UserMgr() = default;

public:
    std::shared_ptr<CSession> GetSession(int uid);

    void SetUserSession(int uid, const std::shared_ptr<CSession>& session);

    void RemoveUserSeesion(int uid);

private:
    std::shared_mutex mtx_;
    std::unordered_map<int, std::shared_ptr<CSession>> sessions_;
};

}  // namespace chat
}  // namespace pyc
