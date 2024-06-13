#include <gtest/gtest.h>

#include "co_async/task.h"
#include "co_async/utils/logger.h"

namespace pyc {
namespace co_async {

static Task<void> void_task() {
    logger.debug("");
    co_return;
}

static Task<std::string> string_task() {
    logger.debug("");
    co_return "aaa";
}

static Task<double> double_task() {
    logger.debug("");
    co_return 3.14;
}

static Task<int> hello() {
    co_await void_task();

    EXPECT_EQ(co_await string_task(), "aaa");

    auto i = static_cast<int>(co_await double_task());
    EXPECT_EQ(i, 3);

    co_return i + 1;
}

TEST(TemplateTaskTest, Basic) {
    auto task = hello();
    task.coroutine_.resume();
    EXPECT_EQ(task.coroutine_.promise().result(), 4);
}

}  // namespace co_async
}  // namespace pyc
