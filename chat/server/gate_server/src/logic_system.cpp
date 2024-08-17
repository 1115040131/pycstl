#include "gate_server/logic_system.h"

#include <nlohmann/json.hpp>

#include "gate_server/define.h"
#include "gate_server/verify_grpc_client.h"

namespace pyc {
namespace chat {

LogicSystem::LogicSystem() {
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->response_.body()) << "receive get_test req\n";
        for (const auto& [key, value] : connection->GetParams()) {
            beast::ostream(connection->response_.body()) << key << ": " << value << "\n";
        }
    });
    RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = beast::buffers_to_string(connection->request_.body().data());
        PYC_LOG_DEBUG("body: {}", body_str);
        connection->response_.set(http::field::content_type, "text/json");

        nlohmann::json src_root = nlohmann::json::parse(body_str, nullptr, false);
        if (src_root.is_discarded()) {
            PYC_LOG_ERROR("Failed to parse Json data");
            nlohmann::json root;
            root["error"] = ErrorCode::kJsonError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        auto email_iter = src_root.find("email");
        if (email_iter == src_root.end()) {
            PYC_LOG_ERROR("Key: email not found");
            nlohmann::json root;
            root["error"] = ErrorCode::kJsonError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        auto email = email_iter->get<std::string>();
        auto response = VerifyGrpcClient::GetInstance().GetVerifyCode(email);

        nlohmann::json root;
        root["email"] = email;
        root["error"] = ErrorCode::kSuccess;
        beast::ostream(connection->response_.body()) << root.dump();
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

void LogicSystem::RegPost(std::string_view url, HttpHandler handler) { post_handlers_.emplace(url, handler); }

bool LogicSystem::HandlePost(std::string_view url, std::shared_ptr<HttpConnection> connection) {
    auto iter = post_handlers_.find(url);
    if (iter == post_handlers_.end()) {
        return false;
    }
    iter->second(connection);
    return true;
}

}  // namespace chat
}  // namespace pyc
