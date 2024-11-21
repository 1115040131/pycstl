#pragma once

#include <QString>

struct SearchInfo {
    int uid;
    int sex;
    QString name;
    QString nick;
    QString icon;
    QString desc;
};

struct ApplyInfo {
    int uid;
    int sex;
    QString name;
    QString nick;
    QString icon;
    QString desc;
    int status = 0;
};

struct AuthInfo {
    int uid;
    int sex;
    QString name;
    QString nick;
    QString icon;
};

struct AuthRsp {
    int uid;
    int sex;
    QString name;
    QString nick;
    QString icon;
};

struct UserInfo {
    int uid;
    int sex;
    QString name;
    QString nick;
    QString icon;

    static UserInfo FromAuthInfo(const AuthInfo& auth_info) {
        return {auth_info.uid, auth_info.sex, auth_info.name, auth_info.nick, auth_info.icon};
    }

    static UserInfo FromAuthRsp(const AuthRsp& auth_rsp) {
        return {auth_rsp.uid, auth_rsp.sex, auth_rsp.name, auth_rsp.nick, auth_rsp.icon};
    }
};