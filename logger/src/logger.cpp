#include "logger/logger.h"

#include <chrono>
#include <cstdint>
#include <mutex>

#include <fmt/chrono.h>
#include <fmt/color.h>

#if defined(__linux__)
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <pthread.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <functional>
#include <thread>
#endif

namespace pyc {

// Returns the OS-level thread id, matching what top/htop/gdb report.
static std::uint64_t OsThreadId() {
    thread_local const std::uint64_t os_thread_id = [] -> std::uint64_t {
#if defined(__linux__)
        return static_cast<std::uint64_t>(::syscall(SYS_gettid));
#elif defined(__APPLE__)
        std::uint64_t tid = 0;
        pthread_threadid_np(nullptr, &tid);
        return tid;
#elif defined(_WIN32)
        return static_cast<std::uint64_t>(::GetCurrentThreadId());
#else
        return std::hash<std::thread::id>{}(std::this_thread::get_id());
#endif
    }();
    return os_thread_id;
}

static consteval std::string_view ToString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::kDebug:
            return "DEBUG";
        case LogLevel::kInfo:
            return "INFO";
        case LogLevel::kWarn:
            return "WARN";
        case LogLevel::kError:
            return "ERROR";
        case LogLevel::kFatal:
            return "FATAL";
    }
    return "UNKNOWN";
}

static consteval fmt::text_style ToTextStyle(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::kDebug:
            return fg(fmt::color::cyan);
        case LogLevel::kInfo:
            return fg(fmt::color::green);
        case LogLevel::kWarn:
            return fg(fmt::color::yellow);
        case LogLevel::kError:
            return fg(fmt::color::red);
        case LogLevel::kFatal:
            return fg(fmt::color::dark_orange) | fmt::emphasis::reverse | fmt::emphasis::bold;
    }
    return {};
}

static constexpr std::string_view ExtractFunctionName(std::string_view full_name) noexcept {
    auto end_pos = full_name.find_last_of('(');
    if (end_pos != std::string::npos) {
        auto start_pos = full_name.rfind("::", end_pos);
        if (start_pos != std::string::npos) {
            start_pos += 2;
        } else {
            start_pos = full_name.rfind(' ', end_pos);
            if (start_pos != std::string::npos) {
                start_pos++;
            } else {
                return full_name.substr(0, end_pos);
            }
        }
        return full_name.substr(start_pos, end_pos - start_pos);
    }
    return full_name;
}

static std::mutex g_log_mutex;

template <LogLevel level>
void Logger::log(std::string_view msg, const std::source_location location) const {
    if (level < min_level_) {
        return;
    }

    std::lock_guard lock(g_log_mutex);
    fmt::print(
        stderr, ToTextStyle(level), "[{}][{:5}][{:0>7}][{:%Y-%m-%d %H:%M:%S}] <{}:{}> [{}] {}\n", name_,
        ToString(level), OsThreadId(),
        std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()}.get_local_time(),
        location.file_name(), location.line(), ExtractFunctionName(location.function_name()), msg);
    if constexpr (level == LogLevel::kFatal) {
        std::fflush(stderr);
    }
}

template void Logger::log<LogLevel::kDebug>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kInfo>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kWarn>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kError>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kFatal>(std::string_view msg, const std::source_location location) const;

}  // namespace pyc
