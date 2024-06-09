#pragma once

#include "design_pattern/filter_strategy.h"
#include "design_pattern/inputer.h"

namespace pyc {

struct StopInputerAdapter : Inputer {
    Inputer* inputer;
    int stop_mark;

    StopInputerAdapter(Inputer* inputer, int stop_mark) : inputer(inputer), stop_mark(stop_mark) {}

    std::optional<int> fetch() override {
        auto tmp = inputer->fetch();
        if (!tmp.has_value() || tmp == stop_mark) {
            return std::nullopt;
        }
        return tmp;
    };
};

struct FilterInputerAdapter : Inputer {
    Inputer* inputer;
    FilterStrategy* filter_strategy;

    FilterInputerAdapter(Inputer* inputer, FilterStrategy* filter_strategy)
        : inputer(inputer), filter_strategy(filter_strategy) {}

    std::optional<int> fetch() override {
        while (true) {
            auto tmp = inputer->fetch();
            if (!tmp.has_value()) {
                return std::nullopt;
            }
            if (filter_strategy->shouldPass(tmp.value())) {
                return tmp;
            }
        }
    }
};

// 跨接口适配器
// 第三方库 api
struct PoostInputer {
    virtual bool hasNext() = 0;
    virtual int getNext() = 0;
};

struct PoostInputerAdapter : Inputer {
    PoostInputer* poost_inputer;

    PoostInputerAdapter(PoostInputer* poost_inputer) : poost_inputer(poost_inputer) {}

    std::optional<int> fetch() override {
        if (poost_inputer->hasNext()) {
            return poost_inputer->getNext();
        }
        return std::nullopt;
    }
};

}  // namespace pyc
