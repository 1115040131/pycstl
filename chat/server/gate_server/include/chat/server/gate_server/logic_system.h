#pragma once

#include <unordered_map>

#include "chat/server/gate_server/http_connection.h"
#include "common/singleton.h"

namespace pyc {
namespace chat {

class LogicSystem : public Singleton<LogicSystem> {
    friend class Singleton<LogicSystem>;

public:
    using HttpHandler = std::function<void(const std::shared_ptr<HttpConnection>&)>;

    ~LogicSystem() = default;

    void RegGet(std::string_view url, HttpHandler handler);

    bool HandleGet(std::string_view url, const std::shared_ptr<HttpConnection>& connection);

    void RegPost(std::string_view url, HttpHandler handler);

    bool HandlePost(std::string_view url, const std::shared_ptr<HttpConnection>& connection);

private:
    LogicSystem();

private:
    std::unordered_map<std::string_view, HttpHandler> get_handlers_;
    std::unordered_map<std::string_view, HttpHandler> post_handlers_;
};

}  // namespace chat
}  // namespace pyc