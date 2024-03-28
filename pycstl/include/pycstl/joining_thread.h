#include <thread>

#include "common/noncopyable.h"

namespace pycstl {

class JoiningThread : pyc::Noncopyable {
public:
    JoiningThread() noexcept = default;

    template <typename Callable, typename... Args>
    explicit JoiningThread(Callable&& func, Args&&... args)
        : thread_(std::forward<Callable>(func), std::forward<Args>(args)...) {}

    explicit JoiningThread(std::thread thread) noexcept : thread_(std::move(thread)) {}

    JoiningThread(JoiningThread&& other) noexcept : thread_(std::move(other.thread_)) {}

    JoiningThread& operator=(JoiningThread&& other) noexcept {
        if (this != &other) {
            if (joinable()) {
                join();
            }
            thread_ = std::move(other.thread_);
        }
        return *this;
    }

    JoiningThread& operator=(std::thread thread) noexcept {
        if (joinable()) {
            join();
        }
        thread_ = std::move(thread);
        return *this;
    }

    ~JoiningThread() noexcept {
        if (joinable()) {
            join();
        }
    }

    void swap(JoiningThread& other) noexcept { thread_.swap(other.thread_); }

    std::thread::id get_id() const noexcept { return thread_.get_id(); }

    bool joinable() const noexcept { return thread_.joinable(); }

    void join() { thread_.join(); }

    void detach() { thread_.detach(); }

    std::thread& as_thread() noexcept { return thread_; }

    const std::thread& as_thread() const noexcept { return thread_; }

private:
    std::thread thread_;
};

}  // namespace pycstl
