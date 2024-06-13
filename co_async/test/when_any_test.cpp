#include <chrono>

#include <gtest/gtest.h>

#include "co_async/sleep.h"
#include "co_async/test/utils.h"
#include "co_async/when_any.h"

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

static Task<int> hello1() {
    Timer timer;
    auto var = co_await when_any(sleep1(), sleep2(), sleep2());
    EXPECT_ELAPSED_TIME(timer.elapsed(), 100ms);

    EXPECT_EQ(var.index(), 0);
    co_return std::get<0>(var);
}

TEST(WhenAnyTest, Hello1) {
    auto task = hello1();
    task.coroutine_.resume();
    TimerLoop().GetInstance().runAll();

    EXPECT_EQ(task.coroutine_.promise().result(), 1);
}

static Task<int> hello2() {
    Timer timer;
    auto var = co_await when_any(sleep2(), sleep1(), sleep2());
    EXPECT_ELAPSED_TIME(timer.elapsed(), 100ms);

    EXPECT_EQ(var.index(), 1);
    co_return std::get<1>(var);
}

TEST(WhenAnyTest, Hello2) {
    auto task = hello2();
    task.coroutine_.resume();
    TimerLoop().GetInstance().runAll();

    EXPECT_EQ(task.coroutine_.promise().result(), 1);
}

}  // namespace co_async
}  // namespace pyc
