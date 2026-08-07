#pragma once

#include <charconv>
#include <chrono>
#include <cstdlib>
#include <string_view>

// 计时断言容差。CI runner 调度抖动大,用 CO_ASYNC_TIMER_TOLERANCE_MS 放宽
// (见 .bazelrc 的 build:ci);默认 1ms,非法值(非数字/负数/有残留)一律回退默认。
inline std::chrono::milliseconds timer_tolerance() {
    using namespace std::chrono_literals;
    static const std::chrono::milliseconds value = [] {
        const char* s = std::getenv("CO_ASYNC_TIMER_TOLERANCE_MS");
        if (!s) {
            return 1ms;
        }
        std::string_view sv{s};
        long ms = 0;
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), ms);
        if (ec != std::errc{} || ptr != sv.data() + sv.size() || ms < 0) {
            return 1ms;
        }
        return std::chrono::milliseconds{ms};
    }();
    return value;
}

class Timer {
public:
    Timer() : start_(std::chrono::system_clock::now()) {}

    std::chrono::system_clock::duration elapsed() const { return std::chrono::system_clock::now() - start_; }

private:
    std::chrono::system_clock::time_point start_;
};

#define EXPECT_ELAPSED_TIME(elapsed_time, expected_time)                                              \
    {                                                                                                 \
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count();   \
        auto expected = std::chrono::duration_cast<std::chrono::milliseconds>(expected_time).count(); \
        auto tolerance = timer_tolerance().count();                                                   \
        EXPECT_NEAR(elapsed, expected, tolerance);                                                    \
    }
