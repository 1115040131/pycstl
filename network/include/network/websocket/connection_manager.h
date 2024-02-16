#pragma once

#include <memory>
#include <unordered_map>

#include "common/singleton.h"
#include "network/websocket/connection.h"

namespace network {

class ConnectionManager : public pyc::Singleton<ConnectionManager> {
public:
    void AddConnection(const std::shared_ptr<Connection>& connection);

    void RemoveConnection(const std::string& uuid);

private:
    std::unordered_map<std::string, std::shared_ptr<Connection>> connections_;
};

}  // namespace network
