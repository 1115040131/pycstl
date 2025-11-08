#pragma once

#include <functional>
#include <tuple>

#include "reaction/concept.h"
#include "reaction/resource.h"

namespace pyc::reaction {

struct VarExpr {};
struct CalcExpr {};

template <typename Fun, typename... Args>
class Expression : public Resource<ReturnType<Fun, Args...>> {
public:
    using ExprType = CalcExpr;
    using ValueType = ReturnType<Fun, Args...>;

    template <typename F, typename... A>
    Expression(F&& f, A&&... args)
        : Resource<ReturnType<Fun, Args...>>(), fun_(std::forward<F>(f)), args_(std::forward<A>(args)...) {
#ifdef USE_FUNCTION
        this->updateObservers([this]() { this->valueChanged(); }, std::forward<A>(args)...);
#else
        this->updateObservers(std::forward<A>(args)...);
#endif
        evaluate();
    }

private:
#ifdef USE_FUNCTION
    void valueChanged() {
        evaluate();
        this->notify();
    }
#else
    void valueChanged() override {
        evaluate();
        this->notify();
    }
#endif

    void evaluate() {
        auto result = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return std::invoke(fun_, std::get<Is>(args_).get().get()...);
        }(std::make_index_sequence<sizeof...(Args)>{});
        this->updateValue(std::move(result));
    }

private:
    Fun fun_;
    std::tuple<std::reference_wrapper<Args>...> args_;
};

template <typename T>
class Expression<T> : public Resource<T> {
public:
    using ExprType = VarExpr;
    using ValueType = T;

    using Resource<T>::Resource;
};

}  // namespace pyc::reaction