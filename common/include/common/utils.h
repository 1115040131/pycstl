#include <type_traits>  // 用于 std::underlying_type

// 转换枚举到其底层类型的通用函数
template <typename E>
constexpr auto ToUnderlying(E e) -> typename std::underlying_type<E>::type {
    return static_cast<typename std::underlying_type<E>::type>(e);
}