#include <chrono>

#include <gtest/gtest.h>

#include "co_async/sleep.h"
#include "co_async/test/utils.h"
#include "co_async/when_all.h"

using namespace std::chrono_literals;

namespace pyc {
namespace co_async {

static Task<int> sleep1() {
    Timer timer;
    co_await sleep_for(100ms);
    EXPECT_ELAPSED_TIME(timer.elapsed(), 100ms);

    co_return 1;
}

static Task<int> sleep2() {
    Timer timer;
    co_await sleep_for(300ms);
    EXPECT_ELAPSED_TIME(timer.elapsed(), 300ms);

    co_return 2;
}

static Task<int> hello() {
    Timer timer;
    auto [i, j, k] = co_await when_all(sleep1(), sleep2(), sleep1());
    EXPECT_ELAPSED_TIME(timer.elapsed(), 300ms);

    co_return i + j + k;
}

TEST(WhenAllTest, Basic) {
    auto task = hello();
    task.coroutine_.resume();
    TimerLoop::GetInstance().runAll();

    EXPECT_EQ(task.coroutine_.promise().result(), 4);
}

}  // namespace co_async
}  // namespace pyc
