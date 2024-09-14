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
            // 创建 schema
            session.createSchema(schema);
            session.sql(fmt::format("USE {}", schema)).execute();
            g_logger.info("Schema '{}' has been created.", schema);

            // 创建 user 表
            session
                .sql(R"(
                    CREATE TABLE IF NOT EXISTS user (
                        id INT NOT NULL AUTO_INCREMENT,
                        uid INT NOT NULL,
                        name VARCHAR(255) NOT NULL,
                        email VARCHAR(255) NOT NULL,
                        pwd VARCHAR(255) NOT NULL,
                        PRIMARY KEY (id)
                    )
                )")
                .execute();
            g_logger.info("Table 'user' has been created.");

            // 创建 user_id 表
            session
                .sql(R"(
                        CREATE TABLE IF NOT EXISTS user_id (
                            id INT NOT NULL
                        )
                    )")
                .execute();

            // 插入一条默认值为0的数据（如果表为空）
            session
                .sql(R"(
                        INSERT INTO user_id (id)
                        SELECT 0 FROM DUAL WHERE NOT EXISTS (SELECT * FROM user_id)
                    )")
                .execute();
            g_logger.info("Table 'user_id' has been created.");

            // 新建存储过程
            session
                .sql(R"(
CREATE DEFINER=`root`@`%` PROCEDURE `reg_user`(
    IN `new_name` VARCHAR(255),
    IN `new_email` VARCHAR(255),
    IN `new_pwd` VARCHAR(255),
    OUT `result` INT)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;
    -- 开始事务
    START TRANSACTION;
    -- 检查用户名是否已存在
    IF EXISTS (SELECT 1 FROM `user` WHERE `name` = new_name) THEN
        SET result = 0; -- 用户名已存在
        COMMIT;
    ELSE
        -- 用户名不存在，检查email是否已存在
        IF EXISTS (SELECT 1 FROM `user` WHERE `email` = new_email) THEN
            SET result = 0; -- email已存在
            COMMIT;
        ELSE
            -- email也不存在，更新user_id表
            UPDATE `user_id` SET `id` = `id` + 1;
            -- 获取更新后的id
            SELECT `id` INTO @new_id FROM `user_id`;
            -- 在user表中插入新记录
            INSERT INTO `user` (`uid`, `name`, `email`, `pwd`) VALUES (@new_id, new_name, new_email, new_pwd);
            -- 设置result为新插入的uid
            SET result = @new_id; -- 插入成功，返回新的uid
            COMMIT;
        END IF;
    END IF;
END
                )")
                .execute();
            g_logger.info("Procedure 'reg_user' has been created.");
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

    } catch (const mysqlx::Error& err) {
        g_logger.fatal("ERROR: {}", err.what());
    } catch (std::exception& ex) {
        g_logger.fatal("STD EXCEPTION: {}", ex.what());
    } catch (const char* ex) {
        g_logger.fatal("EXCEPTION: {}", ex);
    }
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