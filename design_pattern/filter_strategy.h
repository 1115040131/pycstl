#pragma once

namespace pyc {

struct FilterStrategy {
    virtual bool shouldPass(int value) = 0;
};

struct FilterStrategyAbove : FilterStrategy {
    int threshold;

    FilterStrategyAbove(int threshold) : threshold(threshold) {}

    bool shouldPass(int value) override { return value > threshold; }
};

struct FilterStrategyBelow : FilterStrategy {
    int threshold;

    FilterStrategyBelow(int threshold) : threshold(threshold) {}

    bool shouldPass(int value) override { return value < threshold; }
};

struct FilterStrategyAnd : FilterStrategy {
    FilterStrategy* filter1;
    FilterStrategy* filter2;

    FilterStrategyAnd(FilterStrategy* filter1, FilterStrategy* filter2) : filter1(filter1), filter2(filter2) {}

    bool shouldPass(int value) override { return filter1->shouldPass(value) && filter2->shouldPass(value); }
};

}  // namespace pyc
