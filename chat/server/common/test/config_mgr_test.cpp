#include <gtest/gtest.h>

#include "chat/server/common/config_mgr.h"

namespace pyc {
namespace chat {

template <typename... Targs>
void DUMMY_CODE(Targs&&... /* unused */) {}

TEST(ConfigMgrTest, GetConfig) {
    {
        GET_CONFIG(port, "GateServer", "Port");
        EXPECT_EQ(port, "8080");
    }
    {
        GET_CONFIG(host, "VerifyServer", "Host");
        GET_CONFIG(port, "VerifyServer", "Port");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "50051");
    }
    {
        GET_CONFIG(host, "StatusServer", "Host");
        GET_CONFIG(port, "StatusServer", "Port");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "50052");
    }
    {
        GET_CONFIG(name, "ChatServer1", "Name");
        GET_CONFIG(host, "ChatServer1", "Host");
        GET_CONFIG(port, "ChatServer1", "Port");
        GET_CONFIG_INT(rpc_port, "ChatServer1", "RpcPort");
        EXPECT_EQ(name, "ChatServer1");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "8090");
        EXPECT_EQ(rpc_port, 50055);
    }
    {
        GET_CONFIG(name, "ChatServer2", "Name");
        GET_CONFIG(host, "ChatServer2", "Host");
        GET_CONFIG(port, "ChatServer2", "Port");
        GET_CONFIG_INT(rpc_port, "ChatServer2", "RpcPort");
        EXPECT_EQ(name, "ChatServer2");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "8091");
        EXPECT_EQ(rpc_port, 50056);
    }
    {
        GET_CONFIG(host, "Mysql", "Host");
        GET_CONFIG_INT(port, "Mysql", "Port");
        GET_CONFIG(user, "Mysql", "User");
        GET_CONFIG(password, "Mysql", "Password");
        GET_CONFIG(schema, "Mysql", "Schema");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, 33060);
        EXPECT_EQ(user, "root");
        EXPECT_EQ(password, "123456");
        EXPECT_EQ(schema, "pyc_chat");
    }
    {
        GET_CONFIG(host, "Redis", "Host");
        GET_CONFIG_INT(port, "Redis", "Port");
        GET_CONFIG(password, "Redis", "Password");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, 6380);
        EXPECT_EQ(password, "123456");
    }
    EXPECT_EXIT(
        {
            GET_CONFIG(not_exist, "GateServer", "NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT(
        {
            GET_CONFIG(not_exist, "NotExist", "NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT(
        {
            GET_CONFIG_INT(not_exist, "NotExist", "NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::ExitedWithCode(1), "");
}

TEST(ConfigMgrTest, GetSectionConfig) {
    EXPECT_EXIT({ GET_SECTION_CONFIG(not_exist, "NotExist"); }, ::testing::ExitedWithCode(1), "");
    {
        SET_SECTION("GateServer");
        GET_SECTION_CONFIG(port, "Port");
        EXPECT_EQ(port, "8080");
    }
    {
        SET_SECTION("VerifyServer");
        GET_SECTION_CONFIG(host, "Host");
        GET_SECTION_CONFIG(port, "Port");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "50051");
    }
    {
        SET_SECTION("StatusServer");
        GET_SECTION_CONFIG(host, "Host");
        GET_SECTION_CONFIG(port, "Port");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "50052");
    }
    {
        SET_SECTION("ChatServer1");
        GET_SECTION_CONFIG(name, "Name");
        GET_SECTION_CONFIG(host, "Host");
        GET_SECTION_CONFIG(port, "Port");
        GET_SECTION_CONFIG_INT(rpc_port, "RpcPort");
        EXPECT_EQ(name, "ChatServer1");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "8090");
        EXPECT_EQ(rpc_port, 50055);
    }
    {
        SET_SECTION("ChatServer2");
        GET_SECTION_CONFIG(name, "Name");
        GET_SECTION_CONFIG(host, "Host");
        GET_SECTION_CONFIG(port, "Port");
        GET_SECTION_CONFIG_INT(rpc_port, "RpcPort");
        EXPECT_EQ(name, "ChatServer2");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "8091");
        EXPECT_EQ(rpc_port, 50056);
    }
    {
        SET_SECTION("Mysql");
        GET_SECTION_CONFIG(host, "Host");
        GET_SECTION_CONFIG_INT(port, "Port");
        GET_SECTION_CONFIG(user, "User");
        GET_SECTION_CONFIG(password, "Password");
        GET_SECTION_CONFIG(schema, "Schema");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, 33060);
        EXPECT_EQ(user, "root");
        EXPECT_EQ(password, "123456");
        EXPECT_EQ(schema, "pyc_chat");
    }
    {
        SET_SECTION("Redis");
        GET_SECTION_CONFIG(host, "Host");
        GET_SECTION_CONFIG_INT(port, "Port");
        GET_SECTION_CONFIG(password, "Password");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, 6380);
        EXPECT_EQ(password, "123456");
    }
    EXPECT_EXIT({ SET_SECTION("NotExist"); }, ::testing::ExitedWithCode(1), "");
    SET_SECTION("ChatServer1");
    EXPECT_EXIT(
        {
            GET_SECTION_CONFIG(not_exist, "NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT(
        {
            GET_SECTION_CONFIG_INT(not_exist, "NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::ExitedWithCode(1), "");
    EXPECT_EXIT(
        {
            GET_SECTION_CONFIG_INT(cannot_convert, "Name");
            DUMMY_CODE(cannot_convert);
        },
        ::testing::ExitedWithCode(1), "");
}

}  // namespace chat
}  // namespace pyc