#include <gtest/gtest.h>

#include "chat/server/common/redis_mgr.h"

namespace pyc {
namespace chat {

TEST(CommonTest, RedisMgrTest) {
    auto& redis_mgr = RedisMgr::GetInstance();

    EXPECT_TRUE(redis_mgr.Connect("127.0.0.1", 6380));
    EXPECT_TRUE(redis_mgr.Auth("123456"));

    EXPECT_TRUE(redis_mgr.Set("blogwebsite", "llfc.club"));
    EXPECT_EQ(redis_mgr.Get("blogwebsite"), "llfc.club");
    EXPECT_FALSE(redis_mgr.Get("nonekey"));

    EXPECT_TRUE(redis_mgr.HSet("bloginfo", "blogwebsite", "llfc.club"));
    EXPECT_EQ(redis_mgr.HGet("bloginfo", "blogwebsite"), "llfc.club");
    EXPECT_TRUE(redis_mgr.Exists("bloginfo"));
    EXPECT_TRUE(redis_mgr.Del("bloginfo"));
    EXPECT_FALSE(redis_mgr.Del("bloginfo"));
    EXPECT_FALSE(redis_mgr.Exists("bloginfo"));

    EXPECT_TRUE(redis_mgr.LPush("lpushkey1", "lpushvalue1"));
    EXPECT_TRUE(redis_mgr.LPush("lpushkey1", "lpushvalue2"));
    EXPECT_TRUE(redis_mgr.LPush("lpushkey1", "lpushvalue3"));
    EXPECT_EQ(redis_mgr.RPop("lpushkey1"), "lpushvalue1");
    EXPECT_EQ(redis_mgr.RPop("lpushkey1"), "lpushvalue2");
    EXPECT_EQ(redis_mgr.LPop("lpushkey1"), "lpushvalue3");
    EXPECT_FALSE(redis_mgr.LPop("lpushkey2"));

    redis_mgr.Close();
}

}  // namespace chat
}  // namespace pyc
