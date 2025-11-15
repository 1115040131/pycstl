#pragma once

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
    void setSource(F&& fun, A&&... args) {
        static_assert(std::convertible_to<ReturnType<std::decay_t<F>, std::decay_t<A>...>, ValueType>,
                      "ReturnType<std::decay_t<F>, std::decay_t<A>...> should convert to ValueType");

#ifdef USE_FUNCTION
        this->updateObservers([this]() { this->valueChanged(); }, std::forward<A>(args)...);
#else
        this->updateObservers(std::forward<A>(args)...);
#endif
        setFunctor(createFun(std::forward<F>(fun), std::forward<A>(args)...));
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

    template <typename F, typename... A>
    auto createFun(F&& fun, A&&... args) {
        return [fun = std::forward<F>(fun), ... args = args.getPtr()]() {
            if constexpr (VoidType<ValueType>) {
                std::invoke(fun, args->get()...);
                return VoidWrapper{};
            } else {
                return std::invoke(fun, args->get()...);
            }
        };
    }

    void evaluate() {
        if constexpr (VoidType<ValueType>) {
            std::invoke(fun_);
        } else {
            this->updateValue(std::invoke(fun_));
        }
    }

    void setFunctor(const std::function<ValueType()>& fun) { fun_ = fun; }

private:
    std::function<ValueType()> fun_;
};

template <typename T>
class Expression<T> : public Resource<T> {
public:
    using ExprType = VarExpr;
    using ValueType = T;

    using Resource<T>::Resource;
};

}  // namespace pyc::reaction