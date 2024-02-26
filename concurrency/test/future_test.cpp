#include <future>
#include <thread>

#include <gtest/gtest.h>

namespace pyc {
namespace concurrency {

using namespace std::literals::chrono_literals;

int PackagedTask() {
    std::this_thread::sleep_for(50ms);
    return 50;
}

TEST(FutureTest, UsePackagedTask) {
    // 创建一个包装了任务的 packaged_task 对象
    std::packaged_task<int()> task(PackagedTask);
    // 获取与任务关联的 future 对象
    std::future<int> result = task.get_future();

    // 在另一个线程上执行任务
    std::thread t(std::move(task));
    t.detach();  // 将线程与主线程分离

    // 等待任务完成并获取结果
    int value = result.get();
    EXPECT_EQ(value, 50);
}

void SetValue(std::promise<int> prom) {
    std::this_thread::sleep_for(50ms);
    prom.set_value(50);
    // promise 在 set_value 后主线程就可以 get, 之后子线程可以继续执行
}

TEST(FutureTest, UsePromise) {
    // 创建一个 promise 对象
    std::promise<int> prom;
    // 获取与 promise 相关联的 future 对象
    std::future<int> result = prom.get_future();
    // 在新线程中设置 promise
    std::jthread t(SetValue, std::move(prom));
    // 在主线程中获取结果
    EXPECT_EQ(result.get(), 50);
}

void SetException(std::promise<int> prom) {
    try {
        // 抛出异常
        throw std::runtime_error("An error occurred!");
    } catch (...) {
        prom.set_exception(std::current_exception());
    }
}

TEST(FutureTest, UsePromiseException) {
    // 创建一个 promise 对象
    std::promise<int> prom;
    // 获取与 promise 相关联的 future 对象
    std::future<int> result = prom.get_future();
    // 在新线程中设置 promise
    std::jthread t(SetException, std::move(prom));
    // 在主线程中获取结果
    EXPECT_THROW(result.get(), std::runtime_error);
}

TEST(FutureTest, UsePromiseDestruct) {
    std::jthread t;
    std::future<int> result;
    {
        // 创建一个 promise 对象
        std::promise<int> prom;
        // 获取与 promise 相关联的 future 对象
        result = prom.get_future();
        // 在新线程中设置 promise
        t = std::jthread(SetValue, std::move(prom));
    }
    // 在主线程中获取结果
    // prom 所有权被转移到线程中, 不会引发错误
    EXPECT_EQ(result.get(), 50);
}

// 多个线程等待同一个执行结果, 需要使用 shared_future
void ThreadFunction(std::shared_future<int> future) {
    try {
        int result = future.get();
        EXPECT_EQ(result, 50);
    } catch (const std::future_error& e) {
        std::cerr << e.what() << '\n';
    }
}

TEST(FutureTest, UseSharedFuture) {
    std::promise<int> prom;
    std::shared_future<int> future = prom.get_future();
    std::jthread thread(SetValue, std::move(prom));

    constexpr std::size_t kThreadNum = 50;
    std::vector<std::jthread> threads;
    threads.reserve(kThreadNum);
    for (std::size_t i = 0; i < kThreadNum; i++) {
        threads.emplace_back(ThreadFunction, future);
    }
}

}  // namespace concurrency
}  // namespace pyc
