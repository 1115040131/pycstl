#pragma once

#include <source_location>
#include <string>
#include <string_view>

#include <fmt/format.h>

namespace pyc {

enum class LogLevel { kDebug, kInfo, kWarn, kError, kFatal };

class Logger {
public:
    Logger(std::string_view name = "DEFAULT") : name_(name) {}

    template <typename... Args>
    struct FmtWithLocation {
        fmt::format_string<Args...> fmt;
        std::source_location location;

        template <typename S>
        consteval inline FmtWithLocation(const S& fmt,
                                         const std::source_location location = std::source_location::current())
            : fmt(fmt), location(location) {}
    };

    template <typename... Args>
    using FormatString = FmtWithLocation<std::type_identity_t<Args>...>;

    template <typename... Args>
    inline void debug(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log<LogLevel::kDebug>(fmt::format(fmt_with_location.fmt, std::forward<Args>(args)...),
                              fmt_with_location.location);
    }

    template <typename... Args>
    inline void info(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log<LogLevel::kInfo>(fmt::format(fmt_with_location.fmt, std::forward<Args>(args)...),
                             fmt_with_location.location);
    }

    template <typename... Args>
    inline void warn(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log<LogLevel::kWarn>(fmt::format(fmt_with_location.fmt, std::forward<Args>(args)...),
                             fmt_with_location.location);
    }

    template <typename... Args>
    inline void error(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log<LogLevel::kError>(fmt::format(fmt_with_location.fmt, std::forward<Args>(args)...),
                              fmt_with_location.location);
    }

    template <typename... Args>
    inline void fatal(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log<LogLevel::kFatal>(fmt::format(fmt_with_location.fmt, std::forward<Args>(args)...),
                              fmt_with_location.location);
        exit(EXIT_FAILURE);
    }

private:
    template <LogLevel level>
    void log(std::string_view msg, const std::source_location location) const;

private:
    const std::string name_;
};

}  // namespace pyc