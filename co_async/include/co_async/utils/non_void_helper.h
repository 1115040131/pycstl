#pragma once

namespace pyc {
namespace co_async {

template <typename T = void>
struct NonVoidHelper {
    using Type = T;
};

template <>
struct NonVoidHelper<void> {
    using Type = NonVoidHelper;

    explicit NonVoidHelper() noexcept = default;
};

}  // namespace co_async
}  // namespace pyc
