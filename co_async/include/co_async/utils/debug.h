#pragma once

// co_async 的日志只为调试服务。未定义 CO_ASYNC_DEBUG 时,下面的宏展开成空语句,
// logger/ 既不被 import 也不被链接,参数也不会求值。
//
// 开关由构建时的 --config=dbg 决定(见 co_async/BUILD.bazel),不要在
// 某个 .cpp 里单独 #define:协程设施(Promise 等)都定义在头文件里,取值不一致会违反
// ODR,而且已编译进库的 timer_loop.cpp 根本不会跟着变。
//
// 本头含 import 声明,只能 include 进模块的 purview(或普通 TU),不能放进 GMF。
#ifdef CO_ASYNC_DEBUG

import co_async.utils.debug;

#define CO_ASYNC_LOG_DEBUG(...) ::pyc::co_async::DebugLogger().debug(__VA_ARGS__)
#define CO_ASYNC_LOG_INFO(...) ::pyc::co_async::DebugLogger().info(__VA_ARGS__)
#define CO_ASYNC_LOG_WARN(...) ::pyc::co_async::DebugLogger().warn(__VA_ARGS__)
#define CO_ASYNC_LOG_ERROR(...) ::pyc::co_async::DebugLogger().error(__VA_ARGS__)

#else

#define CO_ASYNC_LOG_DEBUG(...) ((void)0)
#define CO_ASYNC_LOG_INFO(...) ((void)0)
#define CO_ASYNC_LOG_WARN(...) ((void)0)
#define CO_ASYNC_LOG_ERROR(...) ((void)0)

#endif
