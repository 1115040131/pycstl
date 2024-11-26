#include "chat/client/user_mgr.h"

#include <algorithm>

bool UserMgr::AlreadyApply(int uid) const {
    return std::ranges::find_if(apply_list_, [uid](const std::shared_ptr<ApplyInfo>& apply) {
               return apply->uid == uid;
           }) != apply_list_.end();
}

void UserMgr::AppendApplyList(const QJsonArray& apply_list) {
    for (const QJsonValue& apply : apply_list) {
        auto apply_info = std::make_shared<ApplyInfo>(
            apply["uid"].toInt(), apply["sex"].toInt(), apply["name"].toString(), apply["nick"].toString(),
            apply["icon"].toString(), apply["desc"].toString(), apply["status"].toInt());
        apply_list_.push_back(apply_info);
    }
}

std::shared_ptr<FriendInfo> UserMgr::GetFriendById(int uid) const {
    auto iter = friend_msp_.find(uid);
    if (iter == friend_msp_.end()) {
        return nullptr;
    }
    return iter->second;
}

bool UserMgr::CheckFriendById(int uid) const { return friend_msp_.find(uid) != friend_msp_.end(); }

void UserMgr::AddFriend(const std::shared_ptr<AuthInfo>& auth_info) {
    auto friend_info = std::make_shared<FriendInfo>(auth_info->uid, auth_info->name, auth_info->nick,
                                                    auth_info->icon, auth_info->sex);
    friend_msp_[friend_info->uid] = friend_info;
}
