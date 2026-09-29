module;

export module co_async.utils.debug;

export import logger.logger;

export namespace pyc::co_async {

// 函数内静态,只有真正开日志的二进制才会构造这个对象。
inline const Logger& DebugLogger() {
    static const Logger logger("co_async");
    return logger;
}

}  // namespace pyc::co_async
