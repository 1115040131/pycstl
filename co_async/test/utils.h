#pragma once

#include <chrono>

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
        auto tolerance = std::chrono::duration_cast<std::chrono::milliseconds>(10ms).count();         \
        EXPECT_NEAR(elapsed, expected, tolerance);                                                    \
    }
