#include <chrono>
#include <coroutine>
#include <map>
#include <unordered_map>

#include "common/singleton.h"

namespace pyc {
namespace co_async {

class TimerLoop : public Singleton<TimerLoop> {
    friend class Singleton<TimerLoop>;

public:
    void addTimer(std::chrono::system_clock::time_point expire_time, std::coroutine_handle<> task);

    void deleteTask(std::coroutine_handle<> task);

    void runAll();

private:
    using TimerMap = std::map<std::chrono::system_clock::time_point, std::coroutine_handle<>>;
    TimerMap timer_map_;
    std::unordered_map<std::coroutine_handle<>, TimerMap::iterator> search_table_;
};

}  // namespace co_async
}  // namespace pyc
