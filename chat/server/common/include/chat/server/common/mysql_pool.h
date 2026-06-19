#pragma once

#include <chrono>
#include <thread>

#include <mysqlx/xdevapi.h>

#include "common/connection_pool.h"

namespace pyc {
namespace chat {

class SqlConnection {
public:
    explicit SqlConnection(
        mysqlx::Session&& session,
        std::chrono::time_point<std::chrono::system_clock> last_use_time = std::chrono::system_clock::now())
        : session_(std::move(session)), last_use_time_(last_use_time) {}

    mysqlx::Session session_;
    std::chrono::time_point<std::chrono::system_clock> last_use_time_;
};

class MysqlPool : public ConnectionPool<SqlConnection> {
public:
    MysqlPool(const std::string& url, int port, const std::string& user, const std::string& password,
              const std::string& schema, size_t size);

private:
    SqlConnection CreateConnection();

    void CheckConnection();

private:
    std::string url_;
    int port_;
    std::string user_;
    std::string password_;
    std::string schema_;

    std::thread check_thread_;  // 防止连接超时
};

}  // namespace chat
}  // namespace pyc

#define MYSQL_CATCH(g_logger)                           \
    catch (const mysqlx::Error& err) {                  \
        g_logger.error("ERROR: {}", err.what());        \
    }                                                   \
    catch (std::exception & ex) {                       \
        g_logger.error("STD EXCEPTION: {}", ex.what()); \
    }                                                   \
    catch (const char* ex) {                            \
        g_logger.error("EXCEPTION: {}", ex);            \
    }
