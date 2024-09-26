#include "chat/server/common/mysql_mgr.h"

#include "chat/server/common/config_mgr.h"
#include "chat/server/common/defer.h"
#include "chat/server/common/mysql_pool.h"

namespace pyc {
namespace chat {

static Logger g_logger("MysqlMgr");

MysqlMgr::MysqlMgr() {
    GET_CONFIG(host, "Mysql", "Host");
    GET_CONFIG_INT(port, "Mysql", "Port");
    GET_CONFIG(user, "Mysql", "User");
    GET_CONFIG(password, "Mysql", "Password");
    GET_CONFIG(schema, "Mysql", "Schema");
    pool_ = std::make_unique<MysqlPool>(host, port, user, password, schema, 5);
}

MysqlMgr::~MysqlMgr() { pool_->Close(); }

std::optional<int> MysqlMgr::RegUser(std::string_view name, std::string_view email, std::string_view password) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        return std::nullopt;
    }
    Defer defer([this, &connection]() {
        connection->last_use_time_ = std::chrono::system_clock::now();
        this->pool_->ReturnConnection(std::move(*connection));
    });

    try {
        connection->session_.sql("CALL reg_user(?, ?, ?, @result)")
            .bind(name.data())
            .bind(email.data())
            .bind(password.data())
            .execute();

        mysqlx::Row row = connection->session_.sql("SELECT @result").execute().fetchOne();

        if (row.isNull()) {
            g_logger.error("Stored procedure not return result");
            return std::nullopt;
        }

        int reg_result = row[0].get<int>();
        g_logger.info("Stored procedure returned: {}", reg_result);
        return reg_result;
    }
    MYSQL_CATCH(g_logger)

    return std::nullopt;
}

std::optional<bool> MysqlMgr::CheckEmail(std::string_view name, std::string_view email) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        return std::nullopt;
    }
    Defer defer([this, &connection]() {
        connection->last_use_time_ = std::chrono::system_clock::now();
        this->pool_->ReturnConnection(std::move(*connection));
    });

    try {
        auto result =
            connection->session_.sql("SELECT email FROM user WHERE name = ?").bind(name.data()).execute();
        if (result.count() == 0) {
            return false;
        }
        return result.fetchOne()[0].get<std::string>() == email;
    }
    MYSQL_CATCH(g_logger)

    return std::nullopt;
}

std::optional<bool> MysqlMgr::UpdatePassword(std::string_view name, std::string_view password) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        return std::nullopt;
    }
    Defer defer([this, &connection]() {
        connection->last_use_time_ = std::chrono::system_clock::now();
        this->pool_->ReturnConnection(std::move(*connection));
    });

    try {
        auto result = connection->session_.sql("UPDATE user SET pwd = ? WHERE name = ? AND pwd <> ?")
                          .bind(password.data())
                          .bind(name.data())
                          .bind(password.data())
                          .execute();
        return result.getAffectedItemsCount() == 1;
    }
    MYSQL_CATCH(g_logger)

    return std::nullopt;
}

std::optional<UserInfo> MysqlMgr::CheckPassword(std::string_view email, std::string_view password) {
    auto connection = pool_->GetConnection();
    if (!connection) {
        return std::nullopt;
    }
    Defer defer([this, &connection]() {
        connection->last_use_time_ = std::chrono::system_clock::now();
        this->pool_->ReturnConnection(std::move(*connection));
    });

    try {
        auto result = connection->session_.sql("SELECT * FROM user WHERE email = ?").bind(email.data()).execute();
        const auto& row = result.fetchOne();
        if (row.isNull() || row[4].get<std::string>() != password) {
            return std::nullopt;
        }

        UserInfo user_info {
            .uid = row[1].get<int>(),
            .name = row[2].get<std::string>(),
            .passward = std::string(password),
            .email = std::string(email),
        };

        return user_info;
    }
    MYSQL_CATCH(g_logger)

    return std::nullopt;
}

}  // namespace chat
}  // namespace pyc
