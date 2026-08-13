#include "logger/sink.h"

#include <fmt/base.h>
#include <fmt/color.h>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace pyc {

static bool IsTerminal(std::FILE* file) {
#if defined(_WIN32)
    return ::_isatty(::_fileno(file)) != 0;
#else
    return ::isatty(::fileno(file)) != 0;
#endif
}

static constexpr fmt::text_style ToTextStyle(LogLevel level) noexcept {
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

FileSink::FileSink(std::FILE* file) : file_(file), colorize_(IsTerminal(file)) {}

void FileSink::WriteImpl(LogLevel level, std::string_view record) {
    if (colorize_) {
        fmt::print(file_, ToTextStyle(level), "{}", record);
        return;
    }
    std::fwrite(record.data(), 1, record.size(), file_);
}

void FileSink::FlushImpl() { std::fflush(file_); }

std::shared_ptr<LogSink> DefaultSink() {
    static const std::shared_ptr<LogSink> sink = std::make_shared<FileSink>(stderr);
    return sink;
}

}  // namespace pyc
