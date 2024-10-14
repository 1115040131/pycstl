#pragma once

#include <QString>

#include "common/singleton.h"

class UserMgr : public pyc::Singleton<UserMgr> {
    friend class pyc::Singleton<UserMgr>;

public:
    void SetUid(int uid) { uid_ = uid; }
    void SetName(const QString& name) { name_ = name; }
    void SetToken(const QString& token) { token_ = token; }

private:
    int uid_;
    QString name_;
    QString token_;
};