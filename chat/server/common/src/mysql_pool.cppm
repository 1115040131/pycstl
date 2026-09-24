module;

#include <chrono>
#include <cstddef>
#include <exception>
#include <source_location>
#include <string>
#include <thread>

#include <mysqlx/xdevapi.h>

#include "common/connection_pool.h"
#include "logger/logger.h"

export module chat.server.common.mysql_pool;

export namespace pyc::chat {

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

// 把 MYSQL_CATCH 的 try/catch 收进函数：只负责捕获并记录，出错时返回什么由调用方自己定。
// 日志位置取自调用点。
template <typename F>
void MysqlCatch(Logger& logger, F&& f, const std::source_location& loc = std::source_location::current()) {
    try {
        f();
    } catch (const mysqlx::Error& err) {
        logger.error_at(loc, "ERROR: {}", err.what());
    } catch (std::exception& ex) {
        logger.error_at(loc, "STD EXCEPTION: {}", ex.what());
    } catch (const char* ex) {
        logger.error_at(loc, "EXCEPTION: {}", ex);
    }
}

}  // namespace pyc::chat
