#pragma once

#include <QJsonArray>
#include <QString>
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
    const QString& GetToken() const { return token_; }
    const std::vector<std::shared_ptr<ApplyInfo>>& GetApplyList() const { return apply_list_; }
    void AddApplyList(int uid, const std::shared_ptr<ApplyInfo>& apply) { apply_list_.push_back(apply); }
    bool AlreadyApply(int uid) const;
    void AppendApplyList(const QJsonArray& apply_list);

private:
    QString token_;
    std::shared_ptr<UserInfo> user_info_;
    std::vector<std::shared_ptr<ApplyInfo>> apply_list_;
};