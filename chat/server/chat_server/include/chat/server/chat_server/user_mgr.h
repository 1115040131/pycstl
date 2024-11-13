#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "common/singleton.h"

namespace pyc {
namespace chat {

class CSession;

class UserMgr : public Singleton<UserMgr> {
    friend class Singleton<UserMgr>;

private:
    UserMgr() = default;

public:
    std::shared_ptr<CSession> GetSession(int uid);

    void SetUserSession(int uid, const std::shared_ptr<CSession>& session);

    void RemoveUserSeesion(int uid);

private:
    std::mutex mtx_;
    std::unordered_map<int, std::shared_ptr<CSession>> sessions_;
};

}  // namespace chat
}  // namespace pyc
