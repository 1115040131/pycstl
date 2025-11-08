#pragma once

#include <memory>

namespace pyc::reaction {

#pragma region Forward declarations

struct VarExpr;

template <typename T, typename... Args>
class ReactImpl;

template <typename ReactType>
class React;

class ObserverNode;

using NodePtr = std::shared_ptr<ObserverNode>;

#pragma endregion

#pragma region Basic concepts

template <typename T, typename U>
concept Convertable = std::is_convertible_v<std::decay_t<T>, std::decay_t<U>>;

template <typename T>
concept IsVarExpr = std::is_same_v<T, VarExpr>;

template <typename T>
concept ConstType = std::is_const_v<std::remove_reference_t<T>>;

template <typename T>
concept VoidType = std::is_void_v<T>;

template <typename T>
concept IsReactNode = requires(T t) {
    { t.shared_from_this() } -> std::same_as<NodePtr>;
};

template <typename T>
concept IsDataReact = requires(T t) {
    typename T::ValueType;
    requires IsReactNode<T> && !VoidType<typename T::ValueType>;
};

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