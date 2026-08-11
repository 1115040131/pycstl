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

// The record is rendered without holding the output path's locks, then handed to each
// sink, which serialises writes on its own mutex. It carries no colour escapes: a sink
// that targets a terminal adds them, which keeps files and pipes clean.
template <LogLevel level>
void Logger::log(std::string_view msg, const std::source_location location) const {
    const auto now = std::chrono::system_clock::now();
    const auto second = std::chrono::floor<std::chrono::seconds>(now);
    const auto fraction = std::chrono::duration_cast<std::chrono::nanoseconds>(now - second).count();

    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "[{}][{:5}][{:0>7}][{}.{:09}] <{}:{}> [{}] {}\n", name_,
                   ToString(level), OsThreadId(), LocalSecondText(second), fraction, location.file_name(),
                   location.line(), ExtractFunctionName(location.function_name()), msg);
    const std::string_view record(buf.data(), buf.size());

    for (const auto& sink : sinks_) {
        sink->Write(level, record);
    }
}

template void Logger::log<LogLevel::kDebug>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kInfo>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kWarn>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kError>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kFatal>(std::string_view msg, const std::source_location location) const;

}  // namespace pyc
