module;

#ifdef CO_ASYNC_DEBUG
#include <utility>
#endif

export module co_async.utils.debug;

#ifdef CO_ASYNC_DEBUG
export import logger.logger;
#endif

export namespace pyc::co_async {

#ifdef CO_ASYNC_DEBUG

inline const Logger& DebugLogger() {
    static const Logger logger("co_async");
    return logger;
}

template <typename... Args>
void LogDebug(Logger::FormatString<Args...> log_fmt, Args&&... args) {
    DebugLogger().debug(log_fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void LogInfo(Logger::FormatString<Args...> log_fmt, Args&&... args) {
    DebugLogger().info(log_fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void LogWarn(Logger::FormatString<Args...> log_fmt, Args&&... args) {
    DebugLogger().warn(log_fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void LogError(Logger::FormatString<Args...> log_fmt, Args&&... args) {
    DebugLogger().error(log_fmt, std::forward<Args>(args)...);
}

#else

// 关日志时的空实现。形参完全泛化, 调用点因此看不到 fmt / logger 的任何类型,
// 这个模块也不会 import logger。代价是实参仍会被求值(原先的宏整句消失, 不求值)。
template <typename... Args>
void LogDebug(Args&&...) {}

template <typename... Args>
void LogInfo(Args&&...) {}

template <typename... Args>
void LogWarn(Args&&...) {}

template <typename... Args>
void LogError(Args&&...) {}

#endif  // CO_ASYNC_DEBUG

}  // namespace pyc::co_async
