#include "gate_server/logic_system.h"

#include <nlohmann/json.hpp>

#include "chat/server/common/redis_mgr.h"
#include "gate_server/define.h"
#include "gate_server/verify_grpc_client.h"

namespace pyc {
namespace chat {

LogicSystem::LogicSystem() {
    RegGet("/get_test", [](const std::shared_ptr<HttpConnection>& connection) {
        beast::ostream(connection->response_.body()) << "receive get_test req\n";
        for (const auto& [key, value] : connection->GetParams()) {
            beast::ostream(connection->response_.body()) << key << ": " << value << "\n";
        }
    });
    RegPost("/get_verifycode", [](const std::shared_ptr<HttpConnection>& connection) {
        auto body_str = beast::buffers_to_string(connection->request_.body().data());
        PYC_LOG_DEBUG("body: {}", body_str);
        connection->response_.set(http::field::content_type, "text/json");

        nlohmann::json src_root = nlohmann::json::parse(body_str, nullptr, false);
        nlohmann::json root;
        if (src_root.is_discarded()) {
            PYC_LOG_ERROR("Failed to parse Json data");
            root["error"] = ErrorCode::kJsonError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        auto email_iter = src_root.find("email");
        if (email_iter == src_root.end()) {
            PYC_LOG_ERROR("Key: email not found");
            root["error"] = ErrorCode::kJsonError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        auto email = email_iter->get<std::string>();
        auto response = VerifyGrpcClient::GetInstance().GetVerifyCode(email);

        root["email"] = email;
        root["error"] = ErrorCode::kSuccess;
        beast::ostream(connection->response_.body()) << root.dump();
    });
    RegPost("/user_register", [](const std::shared_ptr<HttpConnection>& connection) {
        auto body_str = beast::buffers_to_string(connection->request_.body().data());
        PYC_LOG_DEBUG("body: {}", body_str);

        connection->response_.set(http::field::content_type, "text/json");
        nlohmann::json src_root = nlohmann::json::parse(body_str, nullptr, false);
        nlohmann::json root;
        if (src_root.is_discarded()) {
            PYC_LOG_ERROR("Failed to parse Json data");
            root["error"] = ErrorCode::kJsonError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        std::string user = src_root.value("user", "");
        std::string email = src_root.value("email", "");
        std::string password = src_root.value("password", "");
        std::string confirm = src_root.value("confirm", "");
        std::string verify_code = src_root.value("verify_code", "");

        // 确认密码是否一致
        if (password != confirm) {
            PYC_LOG_WARN("{} not match {}", password, confirm);
            root["error"] = ErrorCode::kPasswordError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        // redis 中 email 对应的验证码是否未超时
        auto redis_verify_code = RedisMgr::GetInstance().Get("code_" + email);
        if (!redis_verify_code) {
            PYC_LOG_WARN("{} get verify code expired", email);
            root["error"] = ErrorCode::kVerifyExpired;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        // 验证码是否正确
        if (redis_verify_code.value() != verify_code) {
            PYC_LOG_WARN("{} verify code not match {} {}", email, redis_verify_code.value(), verify_code);
            root["error"] = ErrorCode::kVerifyCodeError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        // TODO: 查找数据库判断用户是否已经注册

        root["error"] = ErrorCode::kSuccess;
        root["user"] = user;
        root["email"] = email;
        root["password"] = password;
        root["confirm"] = confirm;
        root["verify_code"] = verify_code;
        beast::ostream(connection->response_.body()) << root.dump();
    });
}

void LogicSystem::RegGet(std::string_view url, HttpHandler handler) { get_handlers_.emplace(url, handler); }

bool LogicSystem::HandleGet(std::string_view url, const std::shared_ptr<HttpConnection>& connection) {
    auto iter = get_handlers_.find(url);
    if (iter == get_handlers_.end()) {
        return false;
    }
    iter->second(connection);
    return true;
}

void LogicSystem::RegPost(std::string_view url, HttpHandler handler) { post_handlers_.emplace(url, handler); }

bool LogicSystem::HandlePost(std::string_view url, const std::shared_ptr<HttpConnection>& connection) {
    auto iter = post_handlers_.find(url);
    if (iter == post_handlers_.end()) {
        return false;
    }
    iter->second(connection);
    return true;
}

}  // namespace chat
}  // namespace pyc
