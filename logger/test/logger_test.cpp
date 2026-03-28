#include <csignal>

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

TEST(LoggerTest, FatalAborts) {
    Logger logger;
    EXPECT_EXIT(logger.fatal("Hello, World!"), ::testing::KilledBySignal(SIGABRT), "");
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

}  // namespace pyc
