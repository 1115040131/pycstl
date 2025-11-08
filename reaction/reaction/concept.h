#pragma once

#include <type_traits>

namespace pyc::reaction {

#pragma region Forward declarations

struct VarExpr;

template <typename T, typename... Args>
class ReactImpl;

template <typename ReactType>
class React;

#pragma endregion

#pragma region Basic concepts

template <typename T, typename U>
concept Convertable = std::is_convertible_v<std::decay_t<T>, std::decay_t<U>>;

template <typename T>
concept IsVarExpr = std::is_same_v<T, VarExpr>;

#pragma endregion

#pragma region Type traits

template <typename T>
struct ExpressionTraits {
    using type = T;
};

template <typename T>
struct ExpressionTraits<React<ReactImpl<T>>> {
    using type = T;
};

template <typename Fun, typename... Args>
struct ExpressionTraits<React<ReactImpl<Fun, Args...>>> {
    using type = std::invoke_result_t<Fun, typename ExpressionTraits<Args>::type...>;
};

template <typename Fun, typename... Args>
using ReturnType = typename ExpressionTraits<React<ReactImpl<Fun, Args...>>>::type;

#pragma endregion

}  // namespace pyc::reaction