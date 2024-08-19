#include <gtest/gtest.h>

#include "chat/server/common/config_mgr.h"

namespace pyc {
namespace chat {

TEST(CommonTest, ConfigMgrTest) {
    GET_CONFIG(gate_server_port, "GateServer", "Port");
    GET_CONFIG(verify_server_host, "VerifyServer", "Host");
    GET_CONFIG(verify_server_port, "VerifyServer", "Port");
    EXPECT_EQ(gate_server_port, "8080");
    EXPECT_EQ(verify_server_host, "127.0.0.1");
    EXPECT_EQ(verify_server_port, "50051");

    EXPECT_EXIT({ GET_CONFIG(not_exist, "GateServer", "NotExist"); }, ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT({ GET_CONFIG(not_exist, "NotExist", "NotExist"); }, ::testing::ExitedWithCode(1), "");
}

}  // namespace chat
}  // namespace pyc