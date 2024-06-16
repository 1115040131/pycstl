#include <gtest/gtest.h>

#define CO_ASYNC_DEBUG
#include "co_async/task.h"

namespace pyc {
namespace co_async {

static Task<int> world() {
    throw std::runtime_error("world 失败");
    co_return 41;
}

static Task<int> hello() {
    int i = co_await world();
    co_return i + 1;
}

TEST(ExceptionHandleTest, Basic) {
    auto task = hello();
    task.coroutine_.resume();
    EXPECT_THROW(task.coroutine_.promise().result(), std::runtime_error);
}

}  // namespace co_async
}  // namespace pyc
