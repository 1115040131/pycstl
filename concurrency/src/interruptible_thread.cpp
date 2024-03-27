#include "concurrency/interruptible_thread.h"

namespace pyc {
namespace concurrency {

thread_local InterruptFlag this_thread_interrupt_flag;

void InterruptionPoint() {
    if (this_thread_interrupt_flag.IsSet()) {
        throw ThreadInterrupted();
    }
}

void InterruptibleWait(std::condition_variable& cv, std::unique_lock<std::mutex>& lock) {
    InterruptionPoint();
    this_thread_interrupt_flag.SetConditionVariable(cv);
    ClearInterruptFlag guard;
    InterruptionPoint();
    // cv.wait(lock);
    cv.wait_for(lock, std::chrono::milliseconds(1));
    InterruptionPoint();
}

}  // namespace concurrency
}  // namespace pyc
