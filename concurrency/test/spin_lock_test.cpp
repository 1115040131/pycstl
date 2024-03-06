#include <thread>

#include <gtest/gtest.h>

#include "concurrency/spin_lock.h"

namespace pyc {
namespace concurrency {

TEST(SpinLockTest, SpinLockTest) {
    for (int i = 0; i < 100; i++) {
        SpinLock spin_lock;
        std::string result;
        {
            std::jthread t1([&]() {
                spin_lock.Lock();
                for (int i = 0; i < 10; ++i) {
                    result += "*";
                }
                spin_lock.Unlock();
            });
            std::jthread t2([&]() {
                spin_lock.Lock();
                for (int i = 0; i < 10; ++i) {
                    result += "?";
                }
                spin_lock.Unlock();
            });
        }
        EXPECT_TRUE(result == "**********??????????" || result == "??????????**********");
    }
}

}  // namespace concurrency

}  // namespace pyc
