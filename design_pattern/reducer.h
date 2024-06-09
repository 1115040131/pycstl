#pragma once

#include <algorithm>
#include <numeric>

namespace pyc {

struct Reducer {
    virtual int init() = 0;
    virtual int add(int a, int b) = 0;
};

struct SumReducer : Reducer {
    int init() override { return 0; }
    int add(int a, int b) override { return a + b; }
};

struct ProductReducer : Reducer {
    int init() override { return 1; }
    int add(int a, int b) override { return a * b; }
};

struct MinReducer : Reducer {
    int init() override { return std::numeric_limits<int>::max(); }
    int add(int a, int b) override { return std::min(a, b); }
};

struct MaxReducer : Reducer {
    int init() override { return std::numeric_limits<int>::min(); }
    int add(int a, int b) override { return std::max(a, b); }
};

}  // namespace pyc
