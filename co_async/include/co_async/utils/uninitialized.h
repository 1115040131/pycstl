#pragma once

#include "co_async/utils/non_void_helper.h"

namespace pyc {
namespace co_async {

template <typename T>
struct Uninitialized {
    union {
        T value_;
    };

    Uninitialized() noexcept {}
    Uninitialized(Uninitialized&&) = delete;
    ~Uninitialized() noexcept {}

    T moveValue() {
        T value = std::move(value_);
        std::destroy_at(&value_);
        return value;
    }

    template <typename... Args>
    void putValue(Args&&... args) {
        std::construct_at(&value_, std::forward<Args>(args)...);
    }
};

template <>
struct Uninitialized<void> {
    auto moveValue() { return NonVoidHelper<>(); }

    void putValue(NonVoidHelper<>) {}
};

template <typename T>
struct Uninitialized<const T> : Uninitialized<T> {};

template <typename T>
struct Uninitialized<T&> : Uninitialized<std::reference_wrapper<T>> {};

template <typename T>
struct Uninitialized<T&&> : Uninitialized<T> {};

}  // namespace co_async
}  // namespace pyc
