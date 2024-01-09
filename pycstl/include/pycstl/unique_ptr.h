#pragma once

#include <concepts>

#include "pycstl/function.h"

namespace pycstl {

template <typename T, typename U = T>
T exchange(T& val, U&& new_val) {
    T old_val = std::move(val);
    val = std::move(new_val);
    return old_val;
}

template <typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    /// @brief 默认构造函数
    constexpr UniquePtr() = default;

    /// @brief 传入空指针时的构造函数
    constexpr UniquePtr(std::nullptr_t) : UniquePtr() {}

    /// @brief 自定义构造函数
    explicit UniquePtr(T* ptr) : ptr_(ptr) {}

    /// @brief 转换构造函数
    /// @tparam U 基类
    template <typename U, typename UDeleter>
        requires(std::convertible_to<U*, T*>)
    UniquePtr(UniquePtr<U, UDeleter>&& rhs) {
        ptr_ = rhs.release();
    }

    /// @brief 析构函数
    ~UniquePtr() {
        if (ptr_) {
            Deleter{}(ptr_);
        }
    }

    /// @brief 拷贝构造函数
    UniquePtr(const UniquePtr& rhs) = delete;
    /// @brief 拷贝赋值函数
    UniquePtr& operator=(const UniquePtr& rhs) = delete;

    /// @brief 移动构造函数
    UniquePtr(UniquePtr&& rhs) : ptr_(rhs.release()) {}

    /// @brief 移动赋值函数
    UniquePtr& operator=(UniquePtr&& rhs) {
        if (this != &rhs) [[likely]] {
            if (ptr_) {
                Deleter{}(ptr_);
            }
            ptr_ = rhs.release();
        }
        return *this;
    }

    T* get() const { return ptr_; }

    T* release() { return exchange(ptr_, nullptr); }

    void reset(T* ptr = nullptr) {
        if (ptr_) {
            Deleter{}(ptr_);
        }
        ptr_ = ptr;
    }

    T& operator*() const { return *ptr_; }

    T* operator->() const { return ptr_; }

private:
    T* ptr_ = nullptr;
};

/// @brief C 风格数组偏特化
template <typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

template <typename T, typename Deleter>
struct UniquePtr<T[], Deleter> : UniquePtr<T, Deleter> {
};

template <typename T, typename... Args>
UniquePtr<T> makeUnique(Args&&... args) {
    // 由于使用圆括号构造, C++20 之前没有默认构造函数的类无法通过调用该方法
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

/// @brief 不进行零初始化构造, C++20 起
template <typename T>
UniquePtr<T> makeUniqueForOverwrite() {
    return UniquePtr<T>(new T);
}

}  // namespace pycstl
