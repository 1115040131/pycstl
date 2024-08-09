#include "gate_server/logic_system.h"

namespace pyc {
namespace chat {

LogicSystem::LogicSystem() {
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->response_.body()) << "receive get_test req";
    });
}

void LogicSystem::RegGet(std::string_view url, HttpHandler handler) { get_handlers_.emplace(url, handler); }

bool LogicSystem::HandleGet(std::string_view url, std::shared_ptr<HttpConnection> connection) {
    auto iter = get_handlers_.find(url);
    if (iter == get_handlers_.end()) {
        return false;
    }
    iter->second(connection);
    return true;
}

}  // namespace chat
}  // namespace pyc
