#pragma once

#include <string>

namespace pyc {
namespace chat {

struct UserInfo {
    std::string name;
    std::string pwd;
    int uid;
    std::string email;
    std::string nick;
    std::string desc;
    int sex;
    std::string icon;
    std::string back;
};

}  // namespace chat
}  // namespace pyc