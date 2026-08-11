#pragma once

#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <string>
#include <string_view>

#include <fmt/format.h>

namespace pyc {

enum class LogLevel { kDebug, kInfo, kWarn, kError, kFatal };

class Logger {
public:
    explicit Logger(std::string_view name = "DEFAULT") : name_(name) {}

    void set_level(LogLevel level) noexcept { min_level_ = level; }
    LogLevel level() const noexcept { return min_level_; }

    template <typename... Args>
    struct FmtWithLocation {
        fmt::format_string<Args...> fmt;
        std::source_location location;

        template <typename S>
        consteval inline FmtWithLocation(const S& fmt_,
                                         const std::source_location location_ = std::source_location::current())
            : fmt(fmt_), location(location_) {}
    };

    template <typename... Args>
    using FormatString = FmtWithLocation<std::type_identity_t<Args>...>;

    template <typename... Args>
    inline void debug(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log_if<LogLevel::kDebug>(fmt_with_location, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void info(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log_if<LogLevel::kInfo>(fmt_with_location, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void warn(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log_if<LogLevel::kWarn>(fmt_with_location, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void error(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log_if<LogLevel::kError>(fmt_with_location, std::forward<Args>(args)...);
    }

    template <typename... Args>
    [[noreturn]] inline void fatal(FormatString<Args...> fmt_with_location, Args&&... args) const {
        log<LogLevel::kFatal>(fmt::format(fmt_with_location.fmt, std::forward<Args>(args)...),
                              fmt_with_location.location);
        // abort() does not flush stdio streams, so the record would be lost if
        // stderr had been made buffered.
        std::fflush(stderr);
        std::abort();
    }

private:
    // Checks the level before formatting, so filtered-out records cost neither the
    // conversion work nor the std::string allocation.
    template <LogLevel level, typename... Args>
    inline void log_if(const FormatString<Args...>& fmt_with_location, Args&&... args) const {
        if (level < min_level_) {
            return;
        }
        log<level>(fmt::format(fmt_with_location.fmt, std::forward<Args>(args)...), fmt_with_location.location);
    }

    template <LogLevel level>
    void log(std::string_view msg, const std::source_location location) const;

    std::string name_;
    LogLevel min_level_{LogLevel::kDebug};
};

}  // namespace pyc
