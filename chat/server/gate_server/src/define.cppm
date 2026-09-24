module;

#include <utility>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "logger/logger.h"

export module chat.server.gate_server.define;

namespace pyc {
namespace chat {

inline Logger g_logger("GateServer");

}  // namespace chat
}  // namespace pyc

export namespace pyc {
namespace chat {

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

template <typename... Args>
inline void LogDebug(Logger::FormatString<Args...> fmt, Args&&... args) {
    g_logger.debug(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogInfo(Logger::FormatString<Args...> fmt, Args&&... args) {
    g_logger.info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogWarn(Logger::FormatString<Args...> fmt, Args&&... args) {
    g_logger.warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogError(Logger::FormatString<Args...> fmt, Args&&... args) {
    g_logger.error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
[[noreturn]] inline void LogFatal(Logger::FormatString<Args...> fmt, Args&&... args) {
    g_logger.fatal(fmt, std::forward<Args>(args)...);
}

}  // namespace chat
}  // namespace pyc
