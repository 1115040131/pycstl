#pragma once

#include <algorithm>
#include <limits>
#include <memory>

namespace pyc {

struct ReducerState {
    virtual ~ReducerState() = default;
    virtual void add(int val) = 0;
    virtual int result() = 0;
};

struct SumReducerState : ReducerState {
    int res;

    SumReducerState() : res(0) {}

    void add(int val) override { res += val; }
    int result() override { return res; }
};

struct ProductReducerState : ReducerState {
    int res;

    ProductReducerState() : res(1) {}

    void add(int val) override { res *= val; }
    int result() override { return res; }
};

struct MinReducerState : ReducerState {
    int res;

    MinReducerState() : res(std::numeric_limits<int>::max()) {}

    void add(int val) override { res = std::min(res, val); }
    int result() override { return res; }
};

struct MaxReducerState : ReducerState {
    int res;

    MaxReducerState() : res(std::numeric_limits<int>::min()) {}

    void add(int val) override { res = std::max(res, val); }
    int result() override { return res; }
};

struct AverageReducerState : ReducerState {
    int sum;
    int count;

    AverageReducerState() : sum(0), count(0) {}

    void add(int val) override {
        sum += val;
        count++;
    }

    int result() override { return sum / count; }
};

struct Reducer {
    virtual std::unique_ptr<ReducerState> init() = 0;
};

struct SumReducer : Reducer {
    std::unique_ptr<ReducerState> init() override { return std::make_unique<SumReducerState>(); }
};

struct ProductReducer : Reducer {
    std::unique_ptr<ReducerState> init() override { return std::make_unique<ProductReducerState>(); }
};

struct MinReducer : Reducer {
    std::unique_ptr<ReducerState> init() override { return std::make_unique<MinReducerState>(); }
};

struct MaxReducer : Reducer {
    std::unique_ptr<ReducerState> init() override { return std::make_unique<MaxReducerState>(); }
};

struct AverageReducer : Reducer {
    std::unique_ptr<ReducerState> init() override { return std::make_unique<AverageReducerState>(); }
};

}  // namespace pyc
