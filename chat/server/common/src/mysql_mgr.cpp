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

    } catch (const mysqlx::Error& err) {
        g_logger.error("ERROR: {}", err.what());
    } catch (std::exception& ex) {
        g_logger.error("STD EXCEPTION: {}", ex.what());
    } catch (const char* ex) {
        g_logger.error("EXCEPTION: {}", ex);
    }

    return std::nullopt;
}

}  // namespace chat
}  // namespace pyc
