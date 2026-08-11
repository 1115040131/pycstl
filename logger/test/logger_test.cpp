#include <csignal>
#include <cstdio>
#include <latch>
#include <regex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "logger/logger.h"

namespace pyc {

TEST(LoggerTest, OutputContainsLevelAndMessage) {
    Logger logger;

    testing::internal::CaptureStderr();
    logger.debug("Hello, World!");
    std::string out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("DEBUG"));
    EXPECT_TRUE(out.contains("Hello, World!"));

    testing::internal::CaptureStderr();
    logger.info("Hello, World!");
    out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("INFO"));
    EXPECT_TRUE(out.contains("Hello, World!"));

    testing::internal::CaptureStderr();
    logger.warn("Hello, World!");
    out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("WARN"));
    EXPECT_TRUE(out.contains("Hello, World!"));

    testing::internal::CaptureStderr();
    logger.error("Hello, World!");
    out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("ERROR"));
    EXPECT_TRUE(out.contains("Hello, World!"));
}

TEST(LoggerTest, LoggerNameInOutput) {
    Logger logger("MY_LOGGER");
    testing::internal::CaptureStderr();
    logger.info("test message");
    std::string out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("MY_LOGGER"));
}

TEST(LoggerTest, LevelFiltering) {
    Logger logger;
    logger.set_level(LogLevel::kWarn);

    testing::internal::CaptureStderr();
    logger.debug("suppressed");
    logger.info("suppressed");
    std::string out = testing::internal::GetCapturedStderr();
    EXPECT_FALSE(out.contains("suppressed"));

    testing::internal::CaptureStderr();
    logger.warn("visible");
    out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("visible"));
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
    Logger logger;
    testing::internal::CaptureStderr();
    logger.info("{} + {} = {}", 1, 2, 3);
    std::string out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("1 + 2 = 3"));
}

TEST(LoggerTest, FormatFloat) {
    Logger logger;
    testing::internal::CaptureStderr();
    logger.debug("pi = {:.4f}", 3.14159265);
    std::string out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("pi = 3.1416"));
}

TEST(LoggerTest, FormatMixedTypes) {
    Logger logger;
    testing::internal::CaptureStderr();
    logger.warn("user={}, age={}, score={:.1f}", "Alice", 30, 98.6);
    std::string out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("user=Alice, age=30, score=98.6"));
}

TEST(LoggerTest, FormatPadding) {
    Logger logger;
    testing::internal::CaptureStderr();
    logger.info("[{:>10}][{:<10}][{:^10}]", "right", "left", "center");
    std::string out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("[     right]"));
    EXPECT_TRUE(out.contains("[left      ]"));
    EXPECT_TRUE(out.contains("[  center  ]"));
}

TEST(LoggerTest, FormatHex) {
    Logger logger;
    testing::internal::CaptureStderr();
    logger.error("errno=0x{:08X}", 0xDEADBEEF);
    std::string out = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(out.contains("errno=0xDEADBEEF"));
}

// --- Visual / terminal display tests (no assertions, for manual inspection) ---

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
    Logger logger("THREADS");
    constexpr int kThreadCount = 8;

    testing::internal::CaptureStderr();
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
    std::string out = testing::internal::GetCapturedStderr();

    std::set<std::string> ids = ExtractThreadIds(out);
    EXPECT_EQ(ids.size(), static_cast<std::size_t>(kThreadCount));
}

TEST(LoggerTest, MainThreadIdIsStable) {
    Logger logger("THREADS");

    testing::internal::CaptureStderr();
    logger.info("first");
    logger.info("second");
    std::string out = testing::internal::GetCapturedStderr();

    std::set<std::string> ids = ExtractThreadIds(out);
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

}  // namespace pyc
