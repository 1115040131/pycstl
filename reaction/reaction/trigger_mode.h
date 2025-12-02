#pragma once

#include <functional>

namespace pyc::reaction {

struct AlwaysTrig {
    bool checkTrigger() { return true; }
};

struct ChangeTrig {
    bool checkTrigger() { return changed_; }

    void setChanged(bool changed) { changed_ = changed; }

private:
    bool changed_ = true;
};

struct FilterTrig {
    bool checkTrigger() { return std::invoke(filter_fun_); }

    template <typename F, typename... A>
    void filter(F&& fun, A&&... args) {
        filter_fun_ = createFun(std::forward<F>(fun), std::forward<A>(args)...);
    }

private:
    template <typename F, typename... A>
    auto createFun(F&& fun, A&&... args) {
        return
            [fun = std::forward<F>(fun), ... args = args.getPtr()]() { return std::invoke(fun, args->get()...); };
    }

private:
    std::function<bool()> filter_fun_ = []() { return true; };
};

}  // namespace pyc::reaction