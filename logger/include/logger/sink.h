#pragma once

#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

namespace pyc {

enum class LogLevel { kDebug, kInfo, kWarn, kError, kFatal };

// Destination for formatted records.
class LogSink {
public:
    virtual ~LogSink() = default;

    LogSink(const LogSink&) = delete;
    LogSink& operator=(const LogSink&) = delete;

    virtual void Write(LogLevel level, std::string_view record) = 0;

    virtual void Flush() {}

    // Refines the logger's level for this destination: a record has to clear both.
    // The logger checks first and decides whether the record is rendered at all, so a
    // sink can only narrow what its logger already lets through.
    void set_level(LogLevel level) noexcept { min_level_ = level; }
    LogLevel level() const noexcept { return min_level_; }
    bool ShouldWrite(LogLevel level) const noexcept { return level >= min_level_; }

protected:
    LogSink() = default;

private:
    LogLevel min_level_{LogLevel::kDebug};
};

// Stands in for a mutex in a sink that has no state to protect.
struct NullMutex {
    void lock() noexcept {}
    void unlock() noexcept {}
};

template <typename Mutex>
class BaseSink : public LogSink {
public:
    void Write(LogLevel level, std::string_view record) final {
        if (!ShouldWrite(level)) {
            return;
        }
        std::lock_guard lock(mutex_);
        WriteImpl(level, record);
    }

    void Flush() final {
        std::lock_guard lock(mutex_);
        FlushImpl();
    }

protected:
    virtual void WriteImpl(LogLevel level, std::string_view record) = 0;
    virtual void FlushImpl() {}

    mutable Mutex mutex_;
};

enum class ColorMode { kAuto, kAlways, kNever };

// Writes to a C stream. Whether to colour is settled once, on construction, so no record
// pays for the decision.
class FileSink final : public BaseSink<std::mutex> {
public:
    explicit FileSink(std::FILE* file, ColorMode mode = ColorMode::kAuto);

protected:
    void WriteImpl(LogLevel level, std::string_view record) override;
    void FlushImpl() override;

private:
    std::FILE* file_;
    bool colorize_;
};

// Accumulates records in memory, for tests that need to assert on output.
class StringSink final : public BaseSink<std::mutex> {
public:
    // Returns a copy, so the result cannot be read while another thread appends.
    std::string str() const {
        std::lock_guard lock(mutex_);
        return records_;
    }

    void clear() {
        std::lock_guard lock(mutex_);
        records_.clear();
    }

protected:
    void WriteImpl(LogLevel, std::string_view record) override { records_.append(record); }

private:
    std::string records_;
};

// Drops everything. Having no state, it needs no lock, which also makes it a clean
// baseline for measuring what formatting alone costs.
class NullSink final : public BaseSink<NullMutex> {
protected:
    void WriteImpl(LogLevel, std::string_view) override {}
};

// stderr, shared by every logger that does not name its own sinks.
std::shared_ptr<LogSink> DefaultSink();

}  // namespace pyc
