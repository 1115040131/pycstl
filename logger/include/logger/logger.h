#pragma once

#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/base.h>

#include "logger/sink.h"

namespace pyc {

class Logger {
public:
    explicit Logger(std::string_view name = "DEFAULT") : name_(name), sinks_{DefaultSink()} {}

    Logger(std::string_view name, std::shared_ptr<LogSink> sink) : name_(name), sinks_{std::move(sink)} {}

    Logger(std::string_view name, std::initializer_list<std::shared_ptr<LogSink>> sinks)
        : name_(name), sinks_(sinks) {}

    template <typename It>
    Logger(std::string_view name, It first, It last) : name_(name), sinks_(first, last) {}

    void set_level(LogLevel level) noexcept { min_level_ = level; }
    LogLevel level() const noexcept { return min_level_; }

    template <typename... Args>
    struct FmtWithLocation {
        fmt::format_string<Args...> fmt;
        std::source_location location;
        std::string_view function;

        template <typename S>
        consteval inline FmtWithLocation(const S& fmt_,
                                         const std::source_location location_ = std::source_location::current())
            : fmt(fmt_), location(location_), function(ExtractFunctionName(location_.function_name())) {}
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
        log(LogLevel::kFatal, fmt_with_location.fmt.get(), fmt::make_format_args(args...),
            fmt_with_location.location, fmt_with_location.function);
        // abort() does not flush stdio streams, so the record would be lost for any
        // sink that buffers.
        for (const auto& sink : sinks_) {
            sink->Flush();
        }
        std::abort();
    }

private:
    // Trims a __PRETTY_FUNCTION__-style signature down to the bare function name. Runs in
    // FmtWithLocation's consteval constructor, so no record pays for the scan.
    static constexpr std::string_view ExtractFunctionName(std::string_view full_name) noexcept {
        auto end_pos = full_name.find_last_of('(');
        if (end_pos != std::string_view::npos) {
            auto start_pos = full_name.rfind("::", end_pos);
            if (start_pos != std::string_view::npos) {
                start_pos += 2;
            } else {
                start_pos = full_name.rfind(' ', end_pos);
                if (start_pos != std::string_view::npos) {
                    start_pos++;
                } else {
                    return full_name.substr(0, end_pos);
                }
            }
            return full_name.substr(start_pos, end_pos - start_pos);
        }
        return full_name;
    }

    // Checks the level before formatting, so filtered-out records cost neither the
    // conversion work nor the std::string allocation.
    template <LogLevel level, typename... Args>
    inline void log_if(const FormatString<Args...>& fmt_with_location, Args&&... args) const {
        if (level < min_level_) {
            return;
        }
        log(level, fmt_with_location.fmt.get(), fmt::make_format_args(args...), fmt_with_location.location,
            fmt_with_location.function);
    }

    void log(LogLevel level, fmt::string_view format_str, fmt::format_args args,
             const std::source_location location, std::string_view function) const;

    std::string name_;
    std::vector<std::shared_ptr<LogSink>> sinks_;
    LogLevel min_level_{LogLevel::kDebug};
};

}  // namespace pyc
