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

struct TextChatData {
    QString msg_id;
    QString msg_content;
    int from_uid;
    int to_uid;
};

struct FriendInfo {
    int uid;
    QString name;
    QString nick;
    QString icon;
    int sex;
    QString desc;
    QString back;
    QString last_msg;
    std::vector<std::shared_ptr<TextChatData>> chat_msgs;
};

struct UserInfo {
    int uid;
    int sex;
    QString name;
    QString nick;
    QString icon;
    QString last_msg;
    std::vector<std::shared_ptr<TextChatData>> chat_msgs;

    static UserInfo FromSearchInfo(const SearchInfo& search_info) {
        return {search_info.uid, search_info.sex, search_info.name, search_info.nick, search_info.icon};
    }

    static UserInfo FromAuthInfo(const AuthInfo& auth_info) {
        return {auth_info.uid, auth_info.sex, auth_info.name, auth_info.nick, auth_info.icon};
    }

    static UserInfo FromFriendInfo(const FriendInfo& friend_info) {
        return {friend_info.uid,  friend_info.sex,      friend_info.name,     friend_info.nick,
                friend_info.icon, friend_info.last_msg, friend_info.chat_msgs};
    }
};
