#include <condition_variable>
#include <mutex>

#include <gtest/gtest.h>

#include "concurrency/interruptible_thread.h"

namespace pyc {
namespace concurrency {

static std::vector<InterruptibleThread> background_threads;
static std::mutex mtx1;
static std::mutex mtx2;
static std::condition_variable cv1;
static std::condition_variable_any cv2;

void StartBackgroundThreads() {
    background_threads.emplace_back([] {
        try {
            std::unique_lock<std::mutex> lock(mtx1);
            InterruptibleWait(cv1, lock);
        } catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
    });
    background_threads.emplace_back([] {
        try {
            std::unique_lock<std::mutex> lock(mtx2);
            this_thread_interrupt_flag.Wait(cv2, mtx2);
        } catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
    });
}

TEST(InterruptibleThreadTest, InterruptibleWait) {
    StartBackgroundThreads();
    for (auto& thread : background_threads) {
        thread.Interrupt();
    }
    for (auto& thread : background_threads) {
        thread.Join();
    }
}

}  // namespace concurrency
}  // namespace pyc
