#include <thread>

#include <gtest/gtest.h>

#include "concurrency/channel.h"

namespace pyc {
namespace concurrency {

using namespace std::literals::chrono_literals;

TEST(ChannelTest, ChannelTest) {
    Channel<int> ch(10);

    std::jthread producer([&]() {
        for (int i = 0; i < 5; ++i) {
            ch.Send(i);
        }
        ch.Close();
    });

    std::jthread consumer([&]() {
        std::this_thread::sleep_for(10ms);
        int i = 0;
        while (auto val = ch.Receive()) {
            EXPECT_EQ(val, i);
            ++i;
        }
    });
}

}  // namespace concurrency
}  // namespace pyc
