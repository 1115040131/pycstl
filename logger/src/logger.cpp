#include "logger/logger.h"

#include <chrono>

#include <fmt/chrono.h>
#include <fmt/color.h>

#include "common/thread_id.h"

namespace pyc {

consteval inline std::string_view ToString(LogLevel level) noexcept {
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

consteval inline fmt::text_style ToTextStyle(LogLevel level) noexcept {
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

constexpr std::string_view ExtractFunctionName(std::string_view full_name) noexcept {
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

template <LogLevel level>
void Logger::log(std::string_view msg, const std::source_location location) const {
    fmt::print(ToTextStyle(level), "[{}][{:5}][{:0>5}][{:%Y-%m-%d %H:%M:%S}] <{}:{}> [{}] {}\n", name_,
               ToString(level), ShortThreadId(), std::chrono::system_clock::now(), location.file_name(),
               location.line(), ExtractFunctionName(location.function_name()), msg);
}

template void Logger::log<LogLevel::kDebug>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kInfo>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kWarn>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kError>(std::string_view msg, const std::source_location location) const;
template void Logger::log<LogLevel::kFatal>(std::string_view msg, const std::source_location location) const;

}  // namespace pyc