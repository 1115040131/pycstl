#pragma once

#include <QString>
#include <list>
#include <unordered_map>
#include <vector>

#include "chat/client/user_data.h"
#include "common/singleton.h"

class UserMgr : public pyc::Singleton<UserMgr> {
    friend class pyc::Singleton<UserMgr>;

public:
    void SetUserInfo(const std::shared_ptr<UserInfo>& user_info) { user_info_ = user_info; }
    void SetToken(const QString& token) { token_ = token; }

    int GetUid() const { return user_info_->uid; }
    const QString& GetName() const { return user_info_->name; }
    const QString& GetIcon() const { return user_info_->icon; }
    const QString& GetToken() const { return token_; }

    const std::vector<std::shared_ptr<ApplyInfo>>& GetApplyList() const { return apply_list_; }
    void AddApplyList(const std::shared_ptr<ApplyInfo>& apply) { apply_list_.push_back(apply); }
    bool AlreadyApply(int uid) const;

    bool CheckFriendById(int uid) const;
    std::shared_ptr<FriendInfo> GetFriendById(int uid) const;
    void AddFriend(const std::shared_ptr<AuthInfo>& auth_info);
    void AddFriend(const std::shared_ptr<FriendInfo>& friend_info);
    void AppendFriendChatMsg(int friend_id, const std::vector<std::shared_ptr<TextChatData>>& chat_msgs);

    std::vector<std::shared_ptr<FriendInfo>> LoadChatListPerPage();
    bool IsLoadChatFinish() const;
    std::vector<std::shared_ptr<FriendInfo>> LoadContactListPerPage();
    bool IsLoadContactFinish() const;

private:
    static constexpr size_t kChatPerPage = 13;

    QString token_;
    std::shared_ptr<UserInfo> user_info_;
    std::vector<std::shared_ptr<ApplyInfo>> apply_list_;
    std::list<std::shared_ptr<FriendInfo>> friend_list_;
    std::unordered_map<int, std::shared_ptr<FriendInfo>> friend_map_;

    size_t chat_loaded_ = 0;
    size_t contact_loaded_ = 0;
};