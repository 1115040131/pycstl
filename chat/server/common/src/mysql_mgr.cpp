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

template <typename F>
auto MysqlExecute(std::unique_ptr<MysqlPool>& pool,
                  F&& f) -> decltype(f(std::declval<std::optional<SqlConnection>&>())) {
    auto connection = pool->GetConnection();
    if (!connection) {
        return std::nullopt;
    }
    Defer defer([&connection, &pool]() {
        connection->last_use_time_ = std::chrono::system_clock::now();
        pool->ReturnConnection(std::move(*connection));
    });

    try {
        return f(connection);
    }
    MYSQL_CATCH(g_logger)

    return std::nullopt;
}

std::optional<int> MysqlMgr::RegUser(std::string_view name, std::string_view email, std::string_view password) {
    return MysqlExecute(pool_, [=](std::optional<SqlConnection>& connection) -> std::optional<int> {
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
    });
}

std::optional<bool> MysqlMgr::CheckEmail(std::string_view name, std::string_view email) {
    return MysqlExecute(pool_, [=](std::optional<SqlConnection>& connection) -> std::optional<bool> {
        auto result =
            connection->session_.sql("SELECT email FROM user WHERE name = ?").bind(name.data()).execute();
        if (result.count() == 0) {
            return false;
        }
        return result.fetchOne()[0].get<std::string>() == email;
    });
}

std::optional<bool> MysqlMgr::UpdatePassword(std::string_view name, std::string_view password) {
    return MysqlExecute(pool_, [=](std::optional<SqlConnection>& connection) -> std::optional<bool> {
        auto result = connection->session_.sql("UPDATE user SET pwd = ? WHERE name = ? AND pwd <> ?")
                          .bind(password.data())
                          .bind(name.data())
                          .bind(password.data())
                          .execute();
        return result.getAffectedItemsCount() == 1;
    });
}

std::optional<UserInfo> MysqlMgr::CheckPassword(std::string_view email, std::string_view password) {
    return MysqlExecute(pool_, [=](std::optional<SqlConnection>& connection) -> std::optional<UserInfo> {
        auto result = connection->session_.sql("SELECT * FROM user WHERE email = ?").bind(email.data()).execute();
        const auto& row = result.fetchOne();
        if (row.isNull() || row[4].get<std::string>() != password) {
            return std::nullopt;
        }

        UserInfo user_info{};
        user_info.uid = row[1].get<int>();
        user_info.name = row[2].get<std::string>();
        user_info.email = email;
        user_info.password = password;

        return user_info;
    });
}

std::optional<UserInfo> MysqlMgr::GetUser(int uid) {
    return MysqlExecute(pool_, [=](std::optional<SqlConnection>& connection) -> std::optional<UserInfo> {
        auto result = connection->session_.sql("SELECT * FROM user WHERE uid = ?").bind(uid).execute();
        const auto& row = result.fetchOne();
        if (row.isNull()) {
            return std::nullopt;
        }

        UserInfo user_info{};
        user_info.uid = row[1].get<int>();
        user_info.name = row[2].get<std::string>();
        user_info.email = row[3].get<std::string>();
        user_info.password = row[4].get<std::string>();

        return user_info;
    });
}

}  // namespace chat
}  // namespace pyc
