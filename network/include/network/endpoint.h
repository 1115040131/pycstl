#pragma once

namespace network {

/// @brief 创建客户端节点
int ClientEndPoint();

/// @brief 创建服务端节点
int ServerEndPoint();

/// @brief 创建 socket
int CreateTcpSocket();

/// @brief 创建监听 socket
int CreateAcceptorSocket();

/// @brief 创建监听 socket 旧版本写法
int create_acceptor_socket();

/// @brief 绑定acceptor 旧版本写法
int bind_acceptor_socket();

/// @brief 连接指定的端点
int ConnectToEnd();

/// @brief 连接服务器域名
int DnsConnectToEnd();

/// @brief 服务器接收连接
int AcceptNewConnection();

/// @brief 手动构造 asio const buffer
void UseConstBuffer();

/// @brief const buffer 传入 str
void UseBufferStr();

/// @brief const buffer 传入 array
void UseBufferArray();

/// @brief 流式传入 buffer
void UseStreamBuffer();

}  // namespace network
