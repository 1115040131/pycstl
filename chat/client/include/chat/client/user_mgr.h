#pragma once

#include <QString>
#include <vector>

#include "chat/client/user_data.h"
#include "common/singleton.h"

class UserMgr : public pyc::Singleton<UserMgr> {
    friend class pyc::Singleton<UserMgr>;

public:
    void SetUid(int uid) { uid_ = uid; }
    void SetName(const QString& name) { name_ = name; }
    void SetToken(const QString& token) { token_ = token; }

    int GetUid() const { return uid_; }
    const QString& GetName() const { return name_; }
    const QString& GetToken() const { return token_; }
    const std::vector<ApplyInfo>& GetApplyList() const { return apply_list_; }

private:
    int uid_;
    QString name_;
    QString token_;
    std::vector<ApplyInfo> apply_list_;
};