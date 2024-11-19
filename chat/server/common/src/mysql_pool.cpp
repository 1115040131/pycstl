#include "chat/server/common/mysql_pool.h"

#include <fmt/chrono.h>

#include "chat/server/common/defer.h"
#include "logger/logger.h"

namespace pyc {
namespace chat {

using namespace std::chrono_literals;

static Logger g_logger("MysqlMgr");

MysqlPool::MysqlPool(const std::string& url, int port, const std::string& user, const std::string& password,
                     const std::string& schema, size_t size)
    : url_(url), port_(port), user_(user), password_(password), schema_(schema) {
    try {
        // 验证 mysql 版本
        auto session = mysqlx::Session(url_, port_, user_, password_);
        mysqlx::RowResult res = session.sql("show variables like 'version'").execute();
        std::stringstream version;

        version << res.fetchOne().get(1).get<std::string>();
        int major_version;
        version >> major_version;

        if (major_version < 8) {
            g_logger.fatal("Can work only with MySQL Server 8 or later, current_version: {}", major_version);
            return;
        }

        // 如果schema不存在，代表第一次启动, 创建 schema, table, function
        if (!session.getSchema(schema).existsInDatabase()) {
            g_logger.fatal("Schema not exit {}", schema);
        }
        session.close();

        for (size_t i = 0; i < size; i++) {
            connections_.push(CreateConnection());
        }

        check_thread_ = std::thread([this]() {
            while (!stop_) {
                std::this_thread::sleep_for(60s);
                CheckConnection();
            }
        });

        check_thread_.detach();
    }
    MYSQL_CATCH(g_logger)
}

SqlConnection MysqlPool::CreateConnection() {
    auto session = mysqlx::Session(url_, port_, user_, password_);
    // 检查schema是否存在
    if (!session.getSchema(schema_).existsInDatabase()) {
        g_logger.fatal("Schema '{}' create failed", schema_);
    }

    // 设置默认schema为新创建或已存在的schema
    session.sql("USE " + schema_).execute();

    // 从此刻起，这个session对象将以'new_schema'作为默认schema
    // 现在你可以在session中进行数据库操作，操作将默认在'new_schema' schema上执行

    return SqlConnection{std::move(session)};
}

void MysqlPool::CheckConnection() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t size = connections_.size();
    auto now = std::chrono::system_clock::now();

    size_t active = 0;
    size_t inactive = 0;

    for (size_t i = 0; i < size; i++) {
        auto connection = std::move(connections_.front());
        connections_.pop();
        Defer defer([this, &connection]() { this->connections_.push(std::move(connection)); });

        if (now - connection.last_use_time_ < 60s) {
            active++;
            continue;
        }

        try {
            connection.session_.sql("SELECT 1").execute();
            connection.last_use_time_ = now;
            inactive++;
        } catch (const std::exception& e) {
            g_logger.warn("Error keeping connection alive: ", e.what());
            connections_.push(CreateConnection());
        }
    }
    g_logger.debug("cur is {}, total/alive/active/inactive: {}/{}/{}/{}", now, size, active + inactive, active,
                   inactive);
}

}  // namespace chat
}  // namespace pyc