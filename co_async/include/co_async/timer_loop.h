#include <chrono>
#include <coroutine>
#include <map>
#include <unordered_map>

#include "common/singleton.h"

// 检查GCC版本是否小于 13
#if defined(__GNUC__) && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ < 130000)
namespace std {
template <>
struct hash<std::coroutine_handle<>> {
    size_t operator()(const std::coroutine_handle<>& handle) const noexcept {
        return hash<void*>{}(handle.address());
    }
};
}  // namespace std
#endif

namespace pyc {
namespace co_async {

class TimerLoop : public Singleton<TimerLoop> {
    friend class Singleton<TimerLoop>;

public:
    void addTimer(std::chrono::system_clock::time_point expire_time, std::coroutine_handle<> task);

    void deleteTask(std::coroutine_handle<> task);

    void runAll();

private:
    TimerLoop() = default;

private:
    using TimerMap = std::map<std::chrono::system_clock::time_point, std::coroutine_handle<>>;
    TimerMap timer_map_;
    std::unordered_map<std::coroutine_handle<>, TimerMap::iterator> search_table_;
};

}  // namespace co_async
}  // namespace pyc
