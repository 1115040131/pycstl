#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "chat/server/chat_server/define.h"

namespace pyc {
namespace chat {

class CSession;

class CServer {
public:
    CServer(boost::asio::io_context& io_context, const std::string& name, unsigned short port);

    ~CServer();

    void ClearSession(const std::string& uuid);

private:
    void StartAccept();

    void printSessions();

private:
    boost::asio::io_context& io_context_;
    std::string name_; // 服务器名称
    tcp::acceptor acceptor_;
    std::map<std::string, std::shared_ptr<CSession>> sessions_;
    std::mutex mutex_;
};

}  // namespace chat
}  // namespace pyc
