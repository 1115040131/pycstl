#include "chat/server/chat_server/user_mgr.h"

namespace pyc {
namespace chat {

std::shared_ptr<CSession> UserMgr::GetSession(int uid) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto iter = sessions_.find(uid);
    if (iter == sessions_.end()) {
        return nullptr;
    }
    return iter->second;
}

void UserMgr::SetUserSession(int uid, const std::shared_ptr<CSession>& session) {
    std::lock_guard<std::mutex> lock(mtx_);
    sessions_[uid] = session;
}

void UserMgr::RemoveUserSeesion(int uid) {
    std::lock_guard<std::mutex> lock(mtx_);
    sessions_.erase(uid);
}

}  // namespace chat
}  // namespace pyc
