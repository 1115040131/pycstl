#include <fmt/core.h>
#include <gtest/gtest.h>

#include "chat/server/common/data.h"

namespace pyc {
namespace chat {

struct UserInfoAdp1 {
    std::string back;
    std::string nick;
    int sex;
    std::string name;
    std::string password;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserInfoAdp1, back, nick, sex, name, password);
};

TEST(DataTest, UserInfoTest) {
    UserInfo user_info{1,           "user_name", "user_email", "user_password", "user_nick",
                       "user_desc", 2,           "user_icon",  "user_back"};

    {
        nlohmann::json j;
        j["extral"] = "extral";
        j = user_info;  // 覆盖原有的字段

        EXPECT_FALSE(j.contains("extral"));
        EXPECT_EQ(j.get<UserInfo>(), user_info);
    }

    {
        nlohmann::json j;
        j["extral"] = "extral";
        j.update(user_info);  // 不覆盖原有的字段

        EXPECT_EQ(j["extral"].get<std::string>(), "extral");
        EXPECT_EQ(j.get<UserInfo>(), user_info);
    }

    // 兼容性测试
    {
        UserInfoAdp1 user_info_adp1{"user_back", "user_nick", 2, "user_name", "user_password"};

        nlohmann::json j = user_info_adp1;

        auto new_user = j.get<UserInfo>();
        EXPECT_EQ(new_user.back, user_info_adp1.back);
        EXPECT_EQ(new_user.nick, user_info_adp1.nick);
        EXPECT_EQ(new_user.sex, user_info_adp1.sex);
        EXPECT_EQ(new_user.name, user_info_adp1.name);
        EXPECT_EQ(new_user.password, user_info_adp1.password);
    }
}

}  // namespace chat
}  // namespace pyc