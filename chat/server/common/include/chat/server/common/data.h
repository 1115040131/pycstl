#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace pyc {
namespace chat {

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

}  // namespace chat
}  // namespace pyc