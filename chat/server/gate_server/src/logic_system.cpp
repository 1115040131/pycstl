#include "gate_server/logic_system.h"

#include <nlohmann/json.hpp>

#include "chat/common/method_url.h"
#include "chat/server/common/mysql_mgr.h"
#include "chat/server/common/redis_mgr.h"
#include "gate_server/define.h"
#include "gate_server/verify_grpc_client.h"

namespace pyc {
namespace chat {

LogicSystem::LogicSystem() {
    g_logger.info("LogicSystem init start");

    RedisMgr::GetInstance();
    MysqlMgr::GetInstance();

    RegGet("/get_test", [](const std::shared_ptr<HttpConnection>& connection) {
        beast::ostream(connection->response_.body()) << "receive get_test req\n";
        for (const auto& [key, value] : connection->GetParams()) {
            beast::ostream(connection->response_.body()) << key << ": " << value << "\n";
        }
    });
    RegPost(ToUrl(ReqId::kGetVerifyCode), [](const std::shared_ptr<HttpConnection>& connection) {
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
    RegPost(ToUrl(ReqId::kRegUser), [](const std::shared_ptr<HttpConnection>& connection) {
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

        // 查找数据库判断用户是否已经注册
        auto reg_result = MysqlMgr::GetInstance().RegUser(user, email, password);
        if (!reg_result) {
            PYC_LOG_WARN("RegUser({} {} {}) fail", user, email, password);
            root["error"] = ErrorCode::kNetworkError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }
        if (reg_result.value() == 0) {
            PYC_LOG_WARN("User or email already exist: {} {}", user, email);
            root["error"] = ErrorCode::kUserExist;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        PYC_LOG_INFO("RegUser({} {} {}) success, uid: {}", user, email, password, reg_result.value());
        root["error"] = ErrorCode::kSuccess;
        root["user"] = user;
        root["email"] = email;
        root["uid"] = reg_result.value();
        root["password"] = password;
        root["confirm"] = confirm;
        root["verify_code"] = verify_code;
        beast::ostream(connection->response_.body()) << root.dump();
    });
    RegPost(ToUrl(ReqId::kResetPassword), [](const std::shared_ptr<HttpConnection>& connection) {
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
        std::string verify_code = src_root.value("verify_code", "");

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

        // 用户名和邮箱是否匹配
        auto check_email = MysqlMgr::GetInstance().CheckEmail(user, email);
        if (!check_email) {
            PYC_LOG_WARN("CheckEmail({} {}) fail", user, email);
            root["error"] = ErrorCode::kNetworkError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }
        if (!check_email.value()) {
            PYC_LOG_WARN("CheckEmail({} {}) not match", user, email);
            root["error"] = ErrorCode::kEmailNotMatch;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        // 更新密码
        auto update_result = MysqlMgr::GetInstance().UpdatePassword(user, password);
        if (!update_result) {
            PYC_LOG_WARN("UpdatePassword({} {}) fail", user, password);
            root["error"] = ErrorCode::kNetworkError;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }
        if (!update_result.value()) {
            PYC_LOG_WARN("UpdatePassword({} {}) fail", user, password);
            root["error"] = ErrorCode::kPasswordUpdateFail;
            beast::ostream(connection->response_.body()) << root.dump();
            return;
        }

        PYC_LOG_INFO("ResetPwd({} {} {}) success", user, email, password);
        root["error"] = ErrorCode::kSuccess;
        root["user"] = user;
        root["email"] = email;
        root["password"] = password;
        root["verify_code"] = verify_code;
        beast::ostream(connection->response_.body()) << root.dump();
    });

    g_logger.info("LogicSystem init finish");
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
