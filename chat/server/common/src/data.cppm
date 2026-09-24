module;

#include <compare>
#include <string>

#include <nlohmann/json.hpp>

export module chat.server.common.data;

export namespace pyc::chat {

struct UserInfo {
    int uid;
    std::string name;
    std::string email;
    std::string password;
    std::string nick;
    std::string desc;
    int sex;
    std::string icon;
    std::string back;

    auto operator<=>(const UserInfo&) const = default;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserInfo, uid, name, email, password, nick, desc, sex, icon, back);
};

struct ApplyInfo {
    int uid;
    std::string name;
    std::string nick;
    std::string desc;
    int sex;
    std::string icon;
    int status;

    auto operator<=>(const ApplyInfo&) const = default;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ApplyInfo, uid, name, nick, desc, sex, icon, status);
};

}  // namespace pyc::chat
