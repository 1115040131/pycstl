module;

#include <memory>
#include <string>
#include <unordered_map>

#include "common/singleton.h"

export module network.websocket_connection_manager;

export import network.websocket_connection;

export namespace network {

class ConnectionManager : public pyc::Singleton<ConnectionManager> {
    friend class pyc::Singleton<ConnectionManager>;

public:
    void AddConnection(const std::shared_ptr<Connection>& connection);

    void RemoveConnection(const std::string& uuid);

private:
    ConnectionManager() = default;

private:
    std::unordered_map<std::string, std::shared_ptr<Connection>> connections_;
};

}  // namespace network
