#include "logger/logger.h"

#include <chrono>
#include <cstdint>
#include <iterator>
#include <mutex>
#include <string_view>

#include <fmt/chrono.h>
#include <fmt/format.h>

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

// The OS-level thread id, matching what top/htop/gdb report. It never changes for a
// thread, so it is rendered once rather than converted for every record, padding
// included.
static std::string_view OsThreadIdText() {
    thread_local const std::string text = [] {
        const std::uint64_t id = [] -> std::uint64_t {
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
        return fmt::format("{:0>7}", id);
    }();
    return text;
}

static std::mutex g_timestamp_mutex;

static std::string_view LocalSecondText(std::chrono::sys_seconds second) {
    struct Cache {
        std::chrono::sys_seconds second{std::chrono::sys_seconds::min()};
        std::string text;
    };
    thread_local Cache cache;

    if (cache.second != second) {
        static const std::chrono::time_zone* const zone = std::chrono::current_zone();
        std::lock_guard lock(g_timestamp_mutex);
        cache.text.clear();
        fmt::format_to(std::back_inserter(cache.text), "{:%Y-%m-%d %H:%M:%S}",
                       std::chrono::zoned_time{zone, second}.get_local_time());
        cache.second = second;
    }
    return cache.text;
}

static constexpr std::string_view ToString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::kDebug:
            return "DEBUG";
        case LogLevel::kInfo:
            return "INFO ";
        case LogLevel::kWarn:
            return "WARN ";
        case LogLevel::kError:
            return "ERROR";
        case LogLevel::kFatal:
            return "FATAL";
    }
    return "UNKWN";
}

void Logger::log(LogLevel level, fmt::string_view format_str, fmt::format_args args,
                 const std::source_location location, std::string_view function) const {
    const auto now = std::chrono::system_clock::now();
    const auto second = std::chrono::floor<std::chrono::seconds>(now);
    const auto fraction = std::chrono::duration_cast<std::chrono::nanoseconds>(now - second).count();

    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "[{}][{}][{}][{}.{:09}] <{}:{}> [{}] ", name_, ToString(level),
                   OsThreadIdText(), LocalSecondText(second), fraction, location.file_name(), location.line(),
                   function);
    fmt::vformat_to(std::back_inserter(buf), format_str, args);
    buf.push_back('\n');
    const std::string_view record(buf.data(), buf.size());

    for (const auto& sink : sinks_) {
        sink->Write(level, record);
    }
}

}  // namespace pyc
