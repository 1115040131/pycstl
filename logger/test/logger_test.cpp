#include <gtest/gtest.h>

#include "logger/logger.h"

namespace pyc {

TEST(LoggerTest, LogTest) {
    Logger logger;
    logger.debug("{}", "Hello, World!");
    logger.info("{}", "Hello, World!");
    logger.warn("{}", "Hello, World!");
    logger.error("{}", "Hello, World!");
    EXPECT_EXIT(logger.fatal("{}", "Hello, World!"), ::testing::ExitedWithCode(EXIT_FAILURE), "");
}

}  // namespace pyc
