module;

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "common/singleton.h"

export module chat.server.gate_server:logic_system;

export import :http_connection;

export namespace pyc {
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
