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
auto MysqlExecute(std::unique_ptr<MysqlPool>& pool, F&& f) -> decltype(f(std::declval<mysqlx::Session&>())) {
    auto connection = pool->GetConnection();
    if (!connection) {
        return std::nullopt;
    }
    Defer defer([&connection, &pool]() {
        connection->last_use_time_ = std::chrono::system_clock::now();
        pool->ReturnConnection(std::move(*connection));
    });

    try {
        return f(connection->session_);
    }
    MYSQL_CATCH(g_logger)

    return std::nullopt;
}

std::optional<int> MysqlMgr::RegUser(std::string_view name, std::string_view email, std::string_view password) {
    return MysqlExecute(pool_, [=](mysqlx::Session& session) -> std::optional<int> {
        session.sql("CALL reg_user(?, ?, ?, @result)")
            .bind(name.data())
            .bind(email.data())
            .bind(password.data())
            .execute();

        mysqlx::Row row = session.sql("SELECT @result").execute().fetchOne();

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
    return MysqlExecute(pool_, [=](mysqlx::Session& session) -> std::optional<bool> {
        auto result = session.sql("SELECT email FROM user WHERE name = ?").bind(name.data()).execute();
        if (result.count() == 0) {
            return false;
        }
        return result.fetchOne()[0].get<std::string>() == email;
    });
}

std::optional<bool> MysqlMgr::UpdatePassword(std::string_view name, std::string_view password) {
    return MysqlExecute(pool_, [=](mysqlx::Session& session) -> std::optional<bool> {
        auto result = session.sql("UPDATE user SET pwd = ? WHERE name = ? AND pwd <> ?")
                          .bind(password.data())
                          .bind(name.data())
                          .bind(password.data())
                          .execute();
        return result.getAffectedItemsCount() == 1;
    });
}

std::optional<UserInfo> MysqlMgr::CheckPassword(std::string_view email, std::string_view password) {
    return MysqlExecute(pool_, [=](mysqlx::Session& session) -> std::optional<UserInfo> {
        auto result = session.sql("SELECT * FROM user WHERE email = ?").bind(email.data()).execute();
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
    return MysqlExecute(pool_, [=](mysqlx::Session& session) -> std::optional<UserInfo> {
        auto result = session.sql("SELECT * FROM user WHERE uid = ?").bind(uid).execute();
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

std::optional<UserInfo> MysqlMgr::GetUser(std::string_view name) {
    return MysqlExecute(pool_, [=](mysqlx::Session& session) -> std::optional<UserInfo> {
        auto result = session.sql("SELECT * FROM user WHERE name = ?").bind(name.data()).execute();
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

std::optional<bool> MysqlMgr::AddFriendAddply(int from_uid, int to_uid) {
    return MysqlExecute(pool_, [=](mysqlx::Session& session) -> std::optional<bool> {
        auto result = session
                          .sql(
                              "INSERT INTO friend_apply (from_uid, to_uid) values (?, ?) ON DUPLICATE KEY UPDATE "
                              "from_uid = from_uid, to_uid = to_uid")
                          .bind(from_uid)
                          .bind(to_uid)
                          .execute();
        return result.getAffectedItemsCount() > 0;
    });
};

std::optional<std::vector<ApplyInfo>> MysqlMgr::GetApplyList(int to_uid, int begin_index, int limit) {
    return MysqlExecute(pool_, [=](mysqlx::Session& session) -> std::optional<std::vector<ApplyInfo>> {
        auto result = session
                          .sql(
                              "SELECT apply.from_uid, apply.status, user.name, user.nick, user.sex FROM "
                              "friend_apply AS apply JOIN user ON apply.from_uid = user.uid WHERE apply.to_uid = "
                              "? AND apply.id > ? ORDER BY apply.id ASC LIMIT ?")
                          .bind(to_uid)
                          .bind(begin_index)
                          .bind(limit)
                          .execute();

        std::vector<ApplyInfo> apply_list;
        for (const auto& row : result) {
            ApplyInfo apply_info{};
            apply_info.uid = row[0].get<int>();
            apply_info.status = row[1].get<int>();
            apply_info.name = row[2].get<std::string>();
            apply_info.nick = row[3].get<std::string>();
            apply_info.sex = row[4].get<int>();
            apply_list.push_back(std::move(apply_info));
        }
        return apply_list;
    });
}

std::optional<bool> MysqlMgr::DeleteUser(std::string_view email) {
    return MysqlExecute(pool_, [=](mysqlx::Session& session) -> std::optional<bool> {
        auto result = session.sql("DELETE FROM user WHERE email = ?").bind(email.data()).execute();
        return result.getAffectedItemsCount() > 0;
    });
}

}  // namespace chat
}  // namespace pyc
