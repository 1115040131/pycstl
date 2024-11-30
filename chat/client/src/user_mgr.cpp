#include "chat/client/user_mgr.h"

#include <algorithm>

bool UserMgr::AlreadyApply(int uid) const {
    return std::ranges::find_if(apply_list_, [uid](const std::shared_ptr<ApplyInfo>& apply) {
               return apply->uid == uid;
           }) != apply_list_.end();
}

std::shared_ptr<FriendInfo> UserMgr::GetFriendById(int uid) const {
    auto iter = friend_map_.find(uid);
    if (iter == friend_map_.end()) {
        return nullptr;
    }
    return iter->second;
}

bool UserMgr::CheckFriendById(int uid) const { return friend_map_.find(uid) != friend_map_.end(); }

void UserMgr::AddFriend(const std::shared_ptr<AuthInfo>& auth_info) {
    auto friend_info = std::make_shared<FriendInfo>(auth_info->uid, auth_info->name, auth_info->nick,
                                                    auth_info->icon, auth_info->sex);
    AddFriend(friend_info);
}

void UserMgr::AddFriend(const std::shared_ptr<FriendInfo>& friend_info) {
    friend_list_.push_back(friend_info);
    friend_map_[friend_info->uid] = friend_info;
}

void UserMgr::AppendFriendChatMsg(int friend_id, const std::vector<std::shared_ptr<TextChatData>>& chat_msgs) {
    auto iter = friend_map_.find(friend_id);
    if (iter == friend_map_.end()) {
        return;
    }
    iter->second->chat_msgs.insert(iter->second->chat_msgs.end(), chat_msgs.begin(), chat_msgs.end());
}

std::vector<std::shared_ptr<FriendInfo>> UserMgr::LoadChatListPerPage() {
    if (IsLoadChatFinish()) {
        return {};
    }

    auto end = std::min(chat_loaded_ + kChatPerPage, friend_list_.size());
    std::vector<std::shared_ptr<FriendInfo>> chat_list = {std::next(friend_list_.begin(), chat_loaded_),
                                                          std::next(friend_list_.begin(), end)};
    chat_loaded_ = end;
    return chat_list;
}

bool UserMgr::IsLoadChatFinish() const { return chat_loaded_ >= friend_list_.size(); }

std::vector<std::shared_ptr<FriendInfo>> UserMgr::LoadContactListPerPage() {
    if (IsLoadContactFinish()) {
        return {};
    }

    auto end = std::min(contact_loaded_ + kChatPerPage, friend_list_.size());
    std::vector<std::shared_ptr<FriendInfo>> contact_list = {std::next(friend_list_.begin(), contact_loaded_),
                                                             std::next(friend_list_.begin(), end)};
    contact_loaded_ = end;
    return contact_list;
}

bool UserMgr::IsLoadContactFinish() const { return contact_loaded_ >= friend_list_.size(); }
