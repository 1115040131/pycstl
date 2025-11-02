#pragma once

#include <functional>
#include <tuple>
// #include <type_traits>
// #include <utility>

#include "reaction/resource.h"

namespace pyc::reaction {

template <typename T, typename... Args>
class DataSource;

template <typename T>
struct ExpressionTraits {
    using type = T;
};

template <typename T>
struct ExpressionTraits<DataSource<T>> {
    using type = T;
};

template <typename Fun, typename... Args>
struct ExpressionTraits<DataSource<Fun, Args...>> {
    using type = std::invoke_result_t<Fun, typename ExpressionTraits<Args>::type...>;
};

template <typename Fun, typename... Args>
using ReturnType = typename ExpressionTraits<DataSource<Fun, Args...>>::type;

template <typename Fun, typename... Args>
class Expression : public Resource<ReturnType<Fun, Args...>> {
public:
    template <typename F, typename... A>
    Expression(F&& f, A&&... a)
        : Resource<ReturnType<Fun, Args...>>(), fun_(std::forward<F>(f)), args_(std::forward<A>(a)...) {
        evaluate();
    }

private:
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
    using Resource<T>::Resource;
};

}  // namespace pyc::reaction