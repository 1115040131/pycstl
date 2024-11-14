#pragma once

#include <string>

namespace pyc {
namespace chat {

struct UserInfo {
    int uid;
    std::string name;
    std::string email;
    std::string passward;
    std::string nick;
    std::string desc;
    int sex;
    std::string icon;
    std::string back;
};

}  // namespace chat
}  // namespace pyc