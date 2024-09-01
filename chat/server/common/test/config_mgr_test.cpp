#include <gtest/gtest.h>

#include "chat/server/common/config_mgr.h"

namespace pyc {
namespace chat {

TEST(CommonTest, ConfigMgrTest) {
    GET_CONFIG(gate_server_port, "GateServer", "Port");
    EXPECT_EQ(gate_server_port, "8080");

    GET_CONFIG(verify_server_host, "VerifyServer", "Host");
    GET_CONFIG(verify_server_port, "VerifyServer", "Port");
    EXPECT_EQ(verify_server_host, "127.0.0.1");
    EXPECT_EQ(verify_server_port, "50051");

    GET_CONFIG(redis_host, "Redis", "Host");
    GET_CONFIG_INT(redis_port, "Redis", "Port");
    GET_CONFIG(redis_password, "Redis", "Password");
    EXPECT_EQ(redis_host, "127.0.0.1");
    EXPECT_EQ(redis_port, 6380);
    EXPECT_EQ(redis_password, "123456");

    EXPECT_EXIT({ GET_CONFIG(not_exist, "GateServer", "NotExist"); }, ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT({ GET_CONFIG(not_exist, "NotExist", "NotExist"); }, ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT({ GET_CONFIG_INT(not_exist, "NotExist", "NotExist"); }, ::testing::ExitedWithCode(1), "");
}

}  // namespace chat
}  // namespace pyc