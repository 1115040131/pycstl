#include <algorithm>
#include <csignal>
#include <cstdio>
#include <latch>
#include <memory>
#include <regex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "logger/logger.h"

namespace pyc {

// Records are captured through a StringSink rather than by redirecting stderr, so
// the assertions do not depend on gtest internals.
static std::shared_ptr<StringSink> MakeSink() { return std::make_shared<StringSink>(); }

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

// --- Visual / terminal display tests (no assertions, for manual inspection) ---
// These keep the default stderr sink so the output can be eyeballed, including its
// colouring when the test is run from a terminal.

TEST(LoggerTest, VisualAllLevels) {
    Logger logger("DEMO");
    logger.debug("this is a debug message");
    logger.info("this is an info message");
    logger.warn("this is a warning message");
    logger.error("this is an error message");
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

}  // namespace pyc
