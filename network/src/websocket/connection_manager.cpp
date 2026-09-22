module;

#include <memory>
#include <string>

module network.websocket_connection_manager;

namespace network {

void ConnectionManager::AddConnection(const std::shared_ptr<Connection>& connection) {
    connections_[connection->GetUuid()] = connection;
}

void ConnectionManager::RemoveConnection(const std::string& uuid) { connections_.erase(uuid); }

}  // namespace network
