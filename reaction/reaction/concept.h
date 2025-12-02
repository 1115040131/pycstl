#pragma once

#include <memory>

namespace pyc::reaction {

template <typename T>
concept IsTrigMode = requires(T t) {
    { t.checkTrigger() } -> std::same_as<bool>;
};

#pragma region Forward declarations

struct VarExpr;
struct VoidWrapper;

class UniqueID;

template <IsTrigMode TrigMode, typename T, typename... Args>
class ReactImpl;

template <typename ReactType>
class React;

template <typename Type>
struct ValueWrapper;

class ObserverNode;
class FieldBase;

template <typename Op, typename L, typename R>
class BinaryOpExpr;

using NodePtr = std::shared_ptr<ObserverNode>;

#pragma endregion

#pragma region Basic concepts

template <typename T, typename U>
concept Convertable = std::is_convertible_v<std::decay_t<T>, std::decay_t<U>>;

template <typename T>
concept IsVarExpr = std::is_same_v<T, VarExpr>;

template <typename T>
concept HasField = requires(T t) {
    { t.getID() } -> std::same_as<const UniqueID&>;
    requires std::is_base_of_v<FieldBase, std::decay_t<T>>;
};

template <typename T>
concept ConstType = std::is_const_v<std::remove_reference_t<T>>;

template <typename T>
concept VoidType = std::is_void_v<T> || std::is_same_v<T, VoidWrapper>;

template <typename T>
concept InvocableType = std::is_invocable_v<std::decay_t<T>>;

template <typename T>
concept NonInvocableType = !InvocableType<T>;

template <typename... Args>
concept HasArguments = (sizeof...(Args) > 0);

template <typename T>
concept IsReactNode = requires(T t) {
    { t.shared_from_this() } -> std::same_as<NodePtr>;
};

template <typename T>
concept IsDataReact = requires(T t) {
    typename T::ValueType;
    requires IsReactNode<T> && !VoidType<typename T::ValueType>;
};

template <typename T>
concept Cmparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
    { a != b } -> std::convertible_to<bool>;
};

#pragma endregion

#pragma region Type traits

template <typename T>
struct IsReact : std::false_type {};

template <typename T>
struct IsReact<React<T>> : std::true_type {
    using type = T;
};

template <typename T>
struct ExpressionTraits {
    using type = T;
};

template <IsTrigMode TrigMode, NonInvocableType T>
struct ExpressionTraits<React<ReactImpl<TrigMode, T>>> {
    using type = T;
};

template <IsTrigMode TrigMode, typename Fun, typename... Args>
struct ExpressionTraits<React<ReactImpl<TrigMode, Fun, Args...>>> {
    using raw_type = std::invoke_result_t<Fun, typename ExpressionTraits<Args>::type...>;
    using type = std::conditional_t<VoidType<raw_type>, VoidWrapper, raw_type>;
};

template <IsTrigMode TrigMode, typename Fun, typename... Args>
using ReturnType = typename ExpressionTraits<React<ReactImpl<TrigMode, Fun, Args...>>>::type;

template <typename T>
struct BinaryOpExprTraits : std::false_type {};

template <typename Op, typename L, typename R>
struct BinaryOpExprTraits<BinaryOpExpr<Op, L, R>> : std::true_type {};

template <typename T>
concept IsBinaryOpExpr = BinaryOpExprTraits<T>::value;

template <typename T>
using ExprWrapper = std::conditional_t<IsReact<T>::value || IsBinaryOpExpr<T>, T, ValueWrapper<T>>;

template <typename L, typename R>
concept HasCustomOp = IsReact<std::decay_t<L>>::value || IsReact<std::decay_t<R>>::value ||
                      IsBinaryOpExpr<std::decay_t<L>> || IsBinaryOpExpr<std::decay_t<R>>;

#pragma endregion

}  // namespace pyc::reaction