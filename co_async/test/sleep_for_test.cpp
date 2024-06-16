#include <chrono>

#include <gtest/gtest.h>

#include "co_async/sleep.h"
#include "co_async/test/utils.h"

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

TEST(SleepForTest, Basic) {
    auto task1 = sleep1();
    auto task2 = sleep2();

    task1.coroutine_.resume();
    task2.coroutine_.resume();

    Timer timer;
    TimerLoop().GetInstance().runAll();
    EXPECT_ELAPSED_TIME(timer.elapsed(), 300ms);

    EXPECT_EQ(task1.coroutine_.promise().result(), 1);
    EXPECT_EQ(task2.coroutine_.promise().result(), 2);
}

}  // namespace co_async
}  // namespace pyc
