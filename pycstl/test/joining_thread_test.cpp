#include <atomic>
#include <thread>

#include <gtest/gtest.h>

#include "pycstl/joining_thread.h"

namespace pycstl {

static std::atomic<size_t> s_count{0};

static constexpr size_t kLoopTime = 100000;

void Loop() {
    for (size_t i = 0; i < kLoopTime; i++) {
        s_count++;
    }
}

TEST(JoiningThreadTest, JoinTest) {
    {
        s_count = 0;
        JoiningThread jt{Loop};
        EXPECT_TRUE(jt.joinable());
        jt.join();
        EXPECT_EQ(s_count, kLoopTime);
    }

    {
        s_count = 0;
        JoiningThread jt{Loop};
    }

    EXPECT_EQ(s_count, kLoopTime);

    {
        s_count = 0;
        JoiningThread jt{std::thread{Loop}};
    }
    EXPECT_EQ(s_count, kLoopTime);

    {
        s_count = 0;
        JoiningThread jt1{Loop};
        JoiningThread jt2{std::move(jt1)};
    }
    EXPECT_EQ(s_count, kLoopTime);

    {
        s_count = 0;
        JoiningThread jt1{Loop};
        JoiningThread jt2{Loop};
        jt1 = std::move(jt2);
    }
    EXPECT_EQ(s_count, 2 * kLoopTime);

    {
        s_count = 0;
        std::thread t{Loop};
        JoiningThread jt1{Loop};
        jt1 = std::move(t);
    }
    EXPECT_EQ(s_count, 2 * kLoopTime);
}

TEST(JoiningThreadTest, SwapTest) {
    {
        s_count = 0;
        JoiningThread jt1{Loop};
        JoiningThread jt2{Loop};
        jt1.swap(jt2);
    }
    EXPECT_EQ(s_count, 2 * kLoopTime);
}

TEST(JoiningThreadTest, AsThreadTest) {
    JoiningThread jt{};
    EXPECT_EQ(jt.get_id(), jt.as_thread().get_id());
    EXPECT_EQ(jt.joinable(), jt.as_thread().joinable());
}

}  // namespace pycstl