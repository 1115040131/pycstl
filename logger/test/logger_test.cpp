#include <algorithm>
#include <csignal>
#include <cstdio>
#include <fstream>
#include <latch>
#include <memory>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "logger/logger.h"

namespace pyc {

// Records are captured through a StringSink rather than by redirecting stderr, so
// the assertions do not depend on gtest internals.
static std::shared_ptr<StringSink> MakeSink() { return std::make_shared<StringSink>(); }

static std::string TempPath(std::string_view stem) {
    const char* dir = std::getenv("TEST_TMPDIR");
    return std::string(dir != nullptr ? dir : "/tmp") + "/" + std::string(stem);
}

static std::string ReadFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

TEST(LoggerTest, OutputContainsLevelAndMessage) {
    auto sink = MakeSink();
    Logger logger("DEFAULT", sink);

    logger.debug("Hello, World!");
    EXPECT_TRUE(sink->str().contains("DEBUG"));
    EXPECT_TRUE(sink->str().contains("Hello, World!"));

    sink->clear();
    logger.info("Hello, World!");
    EXPECT_TRUE(sink->str().contains("INFO"));
    EXPECT_TRUE(sink->str().contains("Hello, World!"));

    sink->clear();
    logger.warn("Hello, World!");
    EXPECT_TRUE(sink->str().contains("WARN"));
    EXPECT_TRUE(sink->str().contains("Hello, World!"));

    sink->clear();
    logger.error("Hello, World!");
    EXPECT_TRUE(sink->str().contains("ERROR"));
    EXPECT_TRUE(sink->str().contains("Hello, World!"));
}

TEST(LoggerTest, LoggerNameInOutput) {
    auto sink = MakeSink();
    Logger logger("MY_LOGGER", sink);
    logger.info("test message");
    EXPECT_TRUE(sink->str().contains("MY_LOGGER"));
}

TEST(LoggerTest, LevelFiltering) {
    auto sink = MakeSink();
    Logger logger("DEFAULT", sink);
    logger.set_level(LogLevel::kWarn);

    logger.debug("suppressed");
    logger.info("suppressed");
    EXPECT_FALSE(sink->str().contains("suppressed"));

    logger.warn("visible");
    EXPECT_TRUE(sink->str().contains("visible"));
}

// stderr is unbuffered by default, which would hide a missing flush; make it
// buffered so the record only survives if fatal() flushes before aborting, since
// abort() itself does not flush stdio streams.
TEST(LoggerTest, FatalAbortsAndFlushesMessage) {
    EXPECT_EXIT(
        {
            std::setvbuf(stderr, nullptr, _IOFBF, 8192);
            Logger logger;
            logger.fatal("fatal must survive {}", 42);
        },
        ::testing::KilledBySignal(SIGABRT), "fatal must survive 42");
}

// Same contract for a sink the logger was given explicitly, and this time the record is
// read back from outside the process that aborted, so nothing about stderr's buffering
// mode can mask a missing flush.
TEST(LoggerTest, FatalFlushesEveryNamedSink) {
    const std::string path = TempPath("fatal_flush.log");
    EXPECT_EXIT(
        {
            std::FILE* out = std::fopen(path.c_str(), "w");
            std::setvbuf(out, nullptr, _IOFBF, 8192);
            Logger logger("DYING", std::make_shared<FileSink>(out));
            logger.fatal("aborting with {}", 7);
        },
        ::testing::KilledBySignal(SIGABRT), "");

    const std::string written = ReadFile(path);
    EXPECT_TRUE(written.contains("aborting with 7"));
    EXPECT_TRUE(written.contains("FATAL"));
    EXPECT_TRUE(written.contains("DYING"));
}

// --- Format string tests ---

TEST(LoggerTest, FormatIntegers) {
    auto sink = MakeSink();
    Logger logger("DEFAULT", sink);
    logger.info("{} + {} = {}", 1, 2, 3);
    EXPECT_TRUE(sink->str().contains("1 + 2 = 3"));
}

TEST(LoggerTest, FormatFloat) {
    auto sink = MakeSink();
    Logger logger("DEFAULT", sink);
    logger.debug("pi = {:.4f}", 3.14159265);
    EXPECT_TRUE(sink->str().contains("pi = 3.1416"));
}

TEST(LoggerTest, FormatMixedTypes) {
    auto sink = MakeSink();
    Logger logger("DEFAULT", sink);
    logger.warn("user={}, age={}, score={:.1f}", "Alice", 30, 98.6);
    EXPECT_TRUE(sink->str().contains("user=Alice, age=30, score=98.6"));
}

TEST(LoggerTest, FormatPadding) {
    auto sink = MakeSink();
    Logger logger("DEFAULT", sink);
    logger.info("[{:>10}][{:<10}][{:^10}]", "right", "left", "center");
    EXPECT_TRUE(sink->str().contains("[     right]"));
    EXPECT_TRUE(sink->str().contains("[left      ]"));
    EXPECT_TRUE(sink->str().contains("[  center  ]"));
}

TEST(LoggerTest, FormatHex) {
    auto sink = MakeSink();
    Logger logger("DEFAULT", sink);
    logger.error("errno=0x{:08X}", 0xDEADBEEF);
    EXPECT_TRUE(sink->str().contains("errno=0xDEADBEEF"));
}

// --- Call-site metadata tests ---

// Reached directly thanks to -fno-access-control on this target. Going through info() only
// ever evaluates the trimming at compile time, which leaves these rules unverified.
TEST(LoggerTest, ExtractFunctionNameTrimsQualifiersAndParameters) {
    // Qualified: everything up to the last "::" goes, along with the parameter list.
    EXPECT_EQ(Logger::ExtractFunctionName("void pyc::Server::Run(int, char**)"), "Run");
    EXPECT_EQ(Logger::ExtractFunctionName("static std::string_view pyc::Logger::Name()"), "Name");

    // Unqualified: the return type is cut at the last space instead.
    EXPECT_EQ(Logger::ExtractFunctionName("void Run(int)"), "Run");

    // Neither "::" nor a space, so only the parameter list goes.
    EXPECT_EQ(Logger::ExtractFunctionName("Run(int)"), "Run");

    // No parameter list at all: nothing to trim.
    EXPECT_EQ(Logger::ExtractFunctionName("Run"), "Run");
    EXPECT_EQ(Logger::ExtractFunctionName(""), "");
}

static void FreeFunctionLogs(const Logger& logger) { logger.info("from a free function"); }

struct MemberLogger {
    void Emit(const Logger& logger) const { logger.info("from a member function"); }
};

TEST(LoggerTest, RecordCarriesCallSiteFileAndLine) {
    auto sink = MakeSink();
    Logger logger("LOC", sink);

    const int line = __LINE__ + 1;
    logger.info("located");

    EXPECT_TRUE(sink->str().contains("logger_test.cpp:" + std::to_string(line)));
}

// The signature is trimmed in FmtWithLocation's consteval constructor, so what lands in
// the record is the bare name rather than the whole __PRETTY_FUNCTION__ string.
TEST(LoggerTest, FunctionNameIsTrimmedToTheBareName) {
    auto sink = MakeSink();
    Logger logger("LOC", sink);

    logger.info("from a test body");
    EXPECT_TRUE(sink->str().contains("[TestBody]"));

    sink->clear();
    FreeFunctionLogs(logger);
    EXPECT_TRUE(sink->str().contains("[FreeFunctionLogs]"));

    sink->clear();
    MemberLogger{}.Emit(logger);
    EXPECT_TRUE(sink->str().contains("[Emit]"));
}

// --- Visual / terminal display tests (no assertions, for manual inspection) ---
// These write to stderr so the output can be eyeballed, including its colouring when the
// test is run from a terminal.
TEST(LoggerTest, VisualAllLevel) {
    auto sink = std::make_shared<FileSink>(stderr, ColorMode::kAlways);
    Logger logger("COLOR", sink);

    logger.debug("cyan: this is a debug message");
    logger.info("green: this is an info message");
    logger.warn("yellow: this is a warning message");
    logger.error("red: this is an error message");

    const std::string path = TempPath("visual_fatal.log");
    EXPECT_EXIT(
        {
            std::FILE* out = std::fopen(path.c_str(), "w");
            Logger dying("COLOR", std::make_shared<FileSink>(out, ColorMode::kAlways));
            dying.fatal("dark orange, reversed and bold: this is a fatal message");
        },
        ::testing::KilledBySignal(SIGABRT), "");

    const std::string record = ReadFile(path);
    std::fwrite(record.data(), 1, record.size(), stderr);
}

TEST(LoggerTest, VisualFormatting) {
    Logger logger("DEMO");
    logger.info("formatted int: {}, float: {:.2f}, string: {}", 42, 2.718, "hello");
    logger.warn("hex value: 0x{:X}, padded: {:0>8d}", 255, 42);
    logger.debug("vector: [{}, {}, {}]", 1, 2, 3);
}

TEST(LoggerTest, VisualMultiLogger) {
    Logger server("SERVER");
    Logger client("CLIENT");
    Logger db("DATABASE");
    server.info("listening on port {}", 8080);
    client.debug("connecting to {}:{}", "127.0.0.1", 8080);
    db.warn("connection pool at {:.0f}% capacity", 85.0);
    server.error("request timeout after {}ms", 5000);
}

// --- Thread id tests ---

// Extracts the thread-id field (third bracketed group) from each log line.
static std::set<std::string> ExtractThreadIds(const std::string& output) {
    std::set<std::string> ids;
    const std::regex pattern(R"(\[[^\]]*\]\[[^\]]*\]\[(\d+)\])");
    for (std::sregex_iterator it(output.begin(), output.end(), pattern), end; it != end; ++it) {
        ids.insert((*it)[1].str());
    }
    return ids;
}

TEST(LoggerTest, DistinctThreadIdsAcrossThreads) {
    auto sink = MakeSink();
    Logger logger("THREADS", sink);
    constexpr int kThreadCount = 8;

    {
        std::latch start{kThreadCount};
        std::vector<std::thread> threads;
        threads.reserve(kThreadCount);
        for (int i = 0; i < kThreadCount; ++i) {
            threads.emplace_back([&logger, &start, i] {
                start.arrive_and_wait();
                logger.info("message from worker {}", i);
            });
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    std::set<std::string> ids = ExtractThreadIds(sink->str());
    EXPECT_EQ(ids.size(), static_cast<std::size_t>(kThreadCount));
}

TEST(LoggerTest, MainThreadIdIsStable) {
    auto sink = MakeSink();
    Logger logger("THREADS", sink);

    logger.info("first");
    logger.info("second");

    std::set<std::string> ids = ExtractThreadIds(sink->str());
    EXPECT_EQ(ids.size(), 1u);
}

TEST(LoggerTest, VisualThreadIds) {
    Logger logger("WORKER");
    constexpr int kThreadCount = 4;

    std::latch start{kThreadCount};
    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&logger, &start, i] {
            start.arrive_and_wait();
            logger.info("hello from thread #{}", i);
        });
    }
    for (auto& t : threads) {
        t.join();
    }
}

// --- Sink tests ---

TEST(SinkTest, RecordsCarryNoColorEscapes) {
    auto sink = MakeSink();
    Logger logger("PLAIN", sink);
    logger.error("no escapes here");
    // Colouring belongs to a terminal-backed sink, so a captured record must be
    // free of escape sequences; otherwise log files and pipes get polluted.
    EXPECT_EQ(sink->str().find('\033'), std::string::npos);
}

TEST(SinkTest, FanOutDeliversToEverySink) {
    auto first = MakeSink();
    auto second = MakeSink();
    Logger logger("FANOUT", {first, second});

    logger.info("delivered {}", 1);
    EXPECT_TRUE(first->str().contains("delivered 1"));
    EXPECT_TRUE(second->str().contains("delivered 1"));
    EXPECT_EQ(first->str(), second->str());
}

TEST(SinkTest, OneSinkSharedBySeveralLoggers) {
    auto sink = MakeSink();
    Logger first("SVC1", sink);
    Logger second("SVC2", sink);

    first.info("from first");
    second.info("from second");
    EXPECT_TRUE(sink->str().contains("SVC1"));
    EXPECT_TRUE(sink->str().contains("SVC2"));
    // str() returns a copy, so the range has to be taken from a single call.
    EXPECT_EQ(std::ranges::count(sink->str(), '\n'), 2);
}

TEST(SinkTest, NullSinkDiscardsAndFilteringStillApplies) {
    Logger logger("NUL", std::make_shared<NullSink>());
    logger.info("goes nowhere");  // must not crash
    EXPECT_EQ(logger.level(), LogLevel::kDebug);
}

TEST(SinkTest, SinkOutlivesTheLoggerThatNamedIt) {
    auto sink = MakeSink();
    {
        Logger logger("TEMP", sink);
        logger.info("written before the logger went away");
    }
    EXPECT_TRUE(sink->str().contains("written before the logger went away"));
}

TEST(SinkTest, IteratorRangeConstructor) {
    std::vector<std::shared_ptr<LogSink>> sinks{MakeSink(), MakeSink()};
    Logger logger("RANGE", sinks.begin(), sinks.end());

    logger.warn("range built");
    for (const auto& sink : sinks) {
        EXPECT_TRUE(static_cast<StringSink*>(sink.get())->str().contains("range built"));
    }
}

TEST(SinkTest, SinkLevelFiltersIndependently) {
    auto sink = MakeSink();
    sink->set_level(LogLevel::kWarn);
    Logger logger("LEVELS", sink);

    logger.debug("below");
    logger.info("below");
    EXPECT_FALSE(sink->str().contains("below"));

    logger.warn("at");
    logger.error("above");
    EXPECT_TRUE(sink->str().contains("at"));
    EXPECT_TRUE(sink->str().contains("above"));
}

TEST(SinkTest, FanOutSinksFilterAtDifferentLevels) {
    auto verbose = MakeSink();
    auto quiet = MakeSink();
    quiet->set_level(LogLevel::kError);
    Logger logger("SPLIT", {verbose, quiet});

    logger.info("chatter");
    logger.error("trouble");

    EXPECT_TRUE(verbose->str().contains("chatter"));
    EXPECT_TRUE(verbose->str().contains("trouble"));
    EXPECT_FALSE(quiet->str().contains("chatter"));
    EXPECT_TRUE(quiet->str().contains("trouble"));
}

// The logger's level runs first and decides whether a record is rendered at all, so a
// sink cannot widen what its logger already dropped.
TEST(SinkTest, SinkCannotWidenTheLoggersLevel) {
    auto sink = MakeSink();
    sink->set_level(LogLevel::kDebug);
    Logger logger("NARROW", sink);
    logger.set_level(LogLevel::kError);

    logger.debug("dropped by the logger");
    EXPECT_FALSE(sink->str().contains("dropped by the logger"));
}

// A sink that derives straight from LogSink instead of going through BaseSink: it gets
// no lock and the base class's do-nothing Flush, and level filtering is its own
// business, since ShouldWrite is the caller's to consult.
class CountingSink final : public LogSink {
public:
    void Write(LogLevel level, std::string_view) override {
        ++count_;
        last_ = level;
    }

    int count() const noexcept { return count_; }
    LogLevel last() const noexcept { return last_; }

private:
    int count_{0};
    LogLevel last_{LogLevel::kDebug};
};

TEST(SinkTest, DerivingStraightFromLogSinkWorks) {
    auto sink = std::make_shared<CountingSink>();
    Logger logger("DIRECT", sink);

    logger.info("one");
    logger.error("two");
    sink->Flush();  // LogSink's default Flush: nothing to do, must not crash.

    EXPECT_EQ(sink->count(), 2);
    EXPECT_EQ(sink->last(), LogLevel::kError);
}

TEST(SinkTest, SinkLevelDefaultsToDebugAndGatesShouldWrite) {
    CountingSink sink;
    EXPECT_EQ(sink.level(), LogLevel::kDebug);
    EXPECT_TRUE(sink.ShouldWrite(LogLevel::kDebug));

    sink.set_level(LogLevel::kError);
    EXPECT_EQ(sink.level(), LogLevel::kError);
    EXPECT_FALSE(sink.ShouldWrite(LogLevel::kWarn));
    EXPECT_TRUE(sink.ShouldWrite(LogLevel::kError));
    EXPECT_TRUE(sink.ShouldWrite(LogLevel::kFatal));
}

TEST(SinkTest, FlushOnMemorySinksKeepsRecords) {
    auto sink = MakeSink();
    Logger logger("FLUSH", sink);

    logger.info("kept");
    sink->Flush();  // BaseSink's default FlushImpl: nothing is buffered.
    EXPECT_TRUE(sink->str().contains("kept"));

    NullSink discarded;
    discarded.Flush();
}

// --- FileSink tests ---

// Reads back through a second handle, so what is asserted is what actually reached the
// file rather than what is still sitting in the stream's buffer.
TEST(SinkTest, FileSinkFlushPushesRecordsOutOfTheStreamBuffer) {
    const std::string path = TempPath("file_sink.log");
    std::FILE* out = std::fopen(path.c_str(), "w");
    ASSERT_NE(out, nullptr);
    std::setvbuf(out, nullptr, _IOFBF, 8192);

    auto sink = std::make_shared<FileSink>(out);
    Logger logger("FILE", sink);
    logger.info("buffered {}", 7);
    EXPECT_EQ(ReadFile(path), "");

    sink->Flush();
    const std::string written = ReadFile(path);
    EXPECT_TRUE(written.contains("buffered 7"));
    // Not a terminal, so the record must stay free of escape sequences.
    EXPECT_EQ(written.find('\033'), std::string::npos);

    sink.reset();
    std::fclose(out);
}

static std::vector<std::string> ExtractSgrSequences(const std::string& text) {
    std::vector<std::string> sequences;
    const std::regex pattern("\033\\[[0-9;]*m");
    for (std::sregex_iterator it(text.begin(), text.end(), pattern), end; it != end; ++it) {
        sequences.push_back(it->str());
    }
    return sequences;
}

// kAlways is what makes this testable without a terminal to hand. kFatal is written straight
// to the sink because Logger::fatal aborts, so its style cannot be observed from inside the
// process that logs it.
TEST(SinkTest, ColorModeAlwaysGivesEachLevelItsOwnStyle) {
    const std::string path = TempPath("colored.log");
    std::FILE* out = std::fopen(path.c_str(), "w");
    ASSERT_NE(out, nullptr);

    auto sink = std::make_shared<FileSink>(out, ColorMode::kAlways);
    Logger logger("COLOR", sink);
    logger.debug("d");
    logger.info("i");
    logger.warn("w");
    logger.error("e");
    sink->Write(LogLevel::kFatal, "boom\n");
    sink->Flush();

    const std::string written = ReadFile(path);
    EXPECT_TRUE(written.contains("DEBUG"));
    EXPECT_TRUE(written.contains("boom"));

    // A colour sequence plus a reset for each of the four levels above, then three for
    // kFatal, whose emphasis is a sequence of its own ahead of the colour.
    constexpr std::size_t kPlainLevels = 4;
    const std::vector<std::string> sequences = ExtractSgrSequences(written);
    ASSERT_EQ(sequences.size(), kPlainLevels * 2 + 3);

    std::set<std::string> colors;
    for (std::size_t i = 0; i < kPlainLevels * 2; i += 2) {
        colors.insert(sequences[i]);
        // Without the reset, the style would bleed into whatever the terminal prints next.
        EXPECT_EQ(sequences[i + 1], "\033[0m");
    }
    EXPECT_EQ(colors.size(), kPlainLevels);

    EXPECT_EQ(sequences[8], "\033[1;7m");  // bold + reverse
    EXPECT_FALSE(colors.contains(sequences[9]));
    EXPECT_EQ(sequences[10], "\033[0m");

    sink.reset();
    std::fclose(out);
}

// The stream here is a file, so this cannot tell kNever apart from kAuto — what it pins down
// is that kNever never colours, which is what would break if the mode test were inverted.
TEST(SinkTest, ColorModeNeverSuppressesColor) {
    const std::string path = TempPath("plain.log");
    std::FILE* out = std::fopen(path.c_str(), "w");
    ASSERT_NE(out, nullptr);

    auto sink = std::make_shared<FileSink>(out, ColorMode::kNever);
    Logger logger("PLAIN", sink);
    logger.error("no escapes here");
    sink->Flush();

    const std::string written = ReadFile(path);
    EXPECT_TRUE(written.contains("no escapes here"));
    EXPECT_EQ(written.find('\033'), std::string::npos);

    sink.reset();
    std::fclose(out);
}

}  // namespace pyc
