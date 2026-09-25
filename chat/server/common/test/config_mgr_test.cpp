#include <gtest/gtest.h>

import chat.server.common.config_mgr;

namespace pyc {
namespace chat {

template <typename... Targs>
void DUMMY_CODE(Targs&&... /* unused */) {}

TEST(ConfigMgrTest, GetConfig) {
    {
        auto port = GetConfigOrDie("GateServer", "Port");
        EXPECT_EQ(port, "8080");
    }
    {
        auto host = GetConfigOrDie("VerifyServer", "Host");
        auto port = GetConfigOrDie("VerifyServer", "Port");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "50051");
    }
    {
        auto host = GetConfigOrDie("StatusServer", "Host");
        auto port = GetConfigOrDie("StatusServer", "Port");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "50052");
    }
    {
        auto name = GetConfigOrDie("ChatServer1", "Name");
        auto host = GetConfigOrDie("ChatServer1", "Host");
        auto port = GetConfigOrDie("ChatServer1", "Port");
        auto rpc_port = GetConfigIntOrDie("ChatServer1", "RpcPort");
        EXPECT_EQ(name, "ChatServer1");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "8090");
        EXPECT_EQ(rpc_port, 50055);
    }
    {
        auto name = GetConfigOrDie("ChatServer2", "Name");
        auto host = GetConfigOrDie("ChatServer2", "Host");
        auto port = GetConfigOrDie("ChatServer2", "Port");
        auto rpc_port = GetConfigIntOrDie("ChatServer2", "RpcPort");
        EXPECT_EQ(name, "ChatServer2");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "8091");
        EXPECT_EQ(rpc_port, 50056);
    }
    {
        auto host = GetConfigOrDie("Mysql", "Host");
        auto port = GetConfigIntOrDie("Mysql", "Port");
        auto user = GetConfigOrDie("Mysql", "User");
        auto password = GetConfigOrDie("Mysql", "Password");
        auto schema = GetConfigOrDie("Mysql", "Schema");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, 33060);
        EXPECT_EQ(user, "root");
        EXPECT_EQ(password, "123456");
        EXPECT_EQ(schema, "pyc_chat");
    }
    {
        auto host = GetConfigOrDie("Redis", "Host");
        auto port = GetConfigIntOrDie("Redis", "Port");
        auto password = GetConfigOrDie("Redis", "Password");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, 6379);
        EXPECT_EQ(password, "123456");
    }
    EXPECT_EXIT(
        {
            auto not_exist = GetConfigOrDie("GateServer", "NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::KilledBySignal(SIGABRT), "");
    EXPECT_EXIT(
        {
            auto not_exist = GetConfigOrDie("NotExist", "NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::KilledBySignal(SIGABRT), "");
    EXPECT_EXIT(
        {
            auto not_exist = GetConfigIntOrDie("NotExist", "NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::KilledBySignal(SIGABRT), "");
}

TEST(ConfigMgrTest, GetSectionConfig) {
    EXPECT_EXIT({ auto not_exist = GetSectionConfigOrDie("NotExist"); }, ::testing::KilledBySignal(SIGABRT), "");
    {
        SetSectionOrDie("GateServer");
        auto port = GetSectionConfigOrDie("Port");
        EXPECT_EQ(port, "8080");
    }
    {
        SetSectionOrDie("VerifyServer");
        auto host = GetSectionConfigOrDie("Host");
        auto port = GetSectionConfigOrDie("Port");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "50051");
    }
    {
        SetSectionOrDie("StatusServer");
        auto host = GetSectionConfigOrDie("Host");
        auto port = GetSectionConfigOrDie("Port");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "50052");
    }
    {
        SetSectionOrDie("ChatServer1");
        auto name = GetSectionConfigOrDie("Name");
        auto host = GetSectionConfigOrDie("Host");
        auto port = GetSectionConfigOrDie("Port");
        auto rpc_port = GetSectionConfigIntOrDie("RpcPort");
        EXPECT_EQ(name, "ChatServer1");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "8090");
        EXPECT_EQ(rpc_port, 50055);
    }
    {
        SetSectionOrDie("ChatServer2");
        auto name = GetSectionConfigOrDie("Name");
        auto host = GetSectionConfigOrDie("Host");
        auto port = GetSectionConfigOrDie("Port");
        auto rpc_port = GetSectionConfigIntOrDie("RpcPort");
        EXPECT_EQ(name, "ChatServer2");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, "8091");
        EXPECT_EQ(rpc_port, 50056);
    }
    {
        SetSectionOrDie("Mysql");
        auto host = GetSectionConfigOrDie("Host");
        auto port = GetSectionConfigIntOrDie("Port");
        auto user = GetSectionConfigOrDie("User");
        auto password = GetSectionConfigOrDie("Password");
        auto schema = GetSectionConfigOrDie("Schema");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, 33060);
        EXPECT_EQ(user, "root");
        EXPECT_EQ(password, "123456");
        EXPECT_EQ(schema, "pyc_chat");
    }
    {
        SetSectionOrDie("Redis");
        auto host = GetSectionConfigOrDie("Host");
        auto port = GetSectionConfigIntOrDie("Port");
        auto password = GetSectionConfigOrDie("Password");
        EXPECT_EQ(host, "127.0.0.1");
        EXPECT_EQ(port, 6379);
        EXPECT_EQ(password, "123456");
    }
    EXPECT_EXIT({ SetSectionOrDie("NotExist"); }, ::testing::KilledBySignal(SIGABRT), "");
    SetSectionOrDie("ChatServer1");
    EXPECT_EXIT(
        {
            auto not_exist = GetSectionConfigOrDie("NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::KilledBySignal(SIGABRT), "");
    EXPECT_EXIT(
        {
            auto not_exist = GetSectionConfigIntOrDie("NotExist");
            DUMMY_CODE(not_exist);
        },
        ::testing::KilledBySignal(SIGABRT), "");
    EXPECT_EXIT(
        {
            auto cannot_convert = GetSectionConfigIntOrDie("Name");
            DUMMY_CODE(cannot_convert);
        },
        ::testing::KilledBySignal(SIGABRT), "");
}

}  // namespace chat
}  // namespace pyc