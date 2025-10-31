#pragma once

#include <exception>

namespace pycstl {

class BadOptionalAccess : public std::exception {
public:
    BadOptionalAccess() = default;
    virtual ~BadOptionalAccess() = default;

    virtual const char* what() const noexcept override { return "BadOptionalAccess"; }
};

inline constexpr struct Nullopt {
    explicit Nullopt() = default;
} nullopt;

inline constexpr struct InPlace {
    explicit InPlace() = default;
} in_place;

template <typename T>
class Optional {
public:
    Optional() noexcept : has_value_(false), nullopt_() {}

    Optional(Nullopt) noexcept : has_value_(false), nullopt_() {}

    template <typename... Args>
    explicit Optional(InPlace, Args&&... args) noexcept : has_value_(true), value_(std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    explicit Optional(InPlace, std::initializer_list<U> ilist, Args&&... args) noexcept
        : has_value_(true), value_(ilist, std::forward<Args>(args)...) {}

    Optional(const T& value) : has_value_(true), value_(value) {}

    Optional(T&& value) noexcept : has_value_(true), value_(std::move(value)) {}

    Optional(const Optional& that) : has_value_(that.has_value_) {
        if (has_value_) {
            // 调用拷贝构造函数
            new (&value_) T(that.value_);  // placement-new 不分配内存, 只是构造对象

            // 调用赋值构造函数 error!!!
            // value_ = that.value_;
        }
    }

    Optional(Optional&& that) noexcept : has_value_(that.has_value_) {
        if (has_value_) {
            new (&value_) T(std::move(that.value_));
        }
    }

    Optional& operator=(Nullopt) noexcept {
        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
        return *this;
    }

    Optional& operator=(const T& value) {
        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
        new (&value_) T(value);
        has_value_ = true;
        return *this;
    }

    Optional& operator=(T&& value) noexcept {
        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
        new (&value_) T(std::move(value));
        has_value_ = true;
        return *this;
    }

    Optional& operator=(const Optional& that) {
        if (this == &that) {
            return *this;
        }

        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
        if (that.has_value_) {
            new (&value_) T(that.value_);
            has_value_ = true;
        }

        return *this;
    }

    Optional& operator=(Optional&& that) noexcept {
        if (this == &that) {
            return *this;
        }

        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
        if (that.has_value_) {
            new (&value_) T(std::move(that.value_));
            that.value_.~T();
            has_value_ = true;
            that.has_value_ = false;
        }

        return *this;
    }

    ~Optional() noexcept {
        if (has_value_) {
            value_.~T();  // 不释放内存, 只是析构
        }
    }

#pragma region 观察器

    // operator->
    const T* operator->() const& noexcept { return &value_; }

    T* operator->() & noexcept { return &value_; }

    // operator*
    const T& operator*() const& noexcept { return value_; }

    T& operator*() & noexcept { return value_; }

    const T&& operator*() const&& noexcept { return std::move(value_); }

    T&& operator*() && noexcept { return std::move(value_); }

    // operator bool
    explicit operator bool() const noexcept { return has_value_; }

    // has_value
    bool has_value() const noexcept { return has_value_; }

    // value
    const T& value() const& {
        if (!has_value_) {
            throw BadOptionalAccess();
        }
        return value_;
    }

    T& value() & {
        if (!has_value_) {
            throw BadOptionalAccess();
        }
        return value_;
    }

    const T&& value() const&& {
        if (!has_value_) {
            throw BadOptionalAccess();
        }
        return std::move(value_);
    }

    T&& value() && {
        if (!has_value_) {
            throw BadOptionalAccess();
        }
        return std::move(value_);
    }

    // value_or
    T value_or(T default_value) const& {
        if (!has_value_) {
            return default_value;
        }
        return value_;
    }

    T value_or(T default_value) && noexcept {
        if (!has_value_) {
            return default_value;
        }
        return std::move(value_);
    }

#pragma endregion

#pragma region 单子式操作

    template <typename F>
    auto and_then(F&& f) {
        using RetType = std::remove_cvref_t<decltype(f(value_))>;
        if (has_value_) {
            return std::forward<F>(f)(value_);
        }
        return RetType{};
    }

    template <typename F>
    auto transform(F&& f) -> Optional<std::remove_cvref_t<std::invoke_result_t<F, T>>> {
        if (has_value_) {
            return std::forward<F>(f)(value_);
        }
        return nullopt;
    }

    template <typename F>
    Optional or_else(F&& f) {
        if (has_value_) {
            return *this;
        }
        return std::forward<F>(f)();
    }

#pragma endregion

#pragma region 修改器

    void swap(Optional& that) noexcept {
        if (has_value_ && that.has_value_) {
            std::swap(value_, that.value_);
        } else if (has_value_) {
            that.emplace(std::move(value_));
            that.has_value_ = true;
            reset();
        } else if (that.has_value_) {
            emplace(std::move(that.value_));
            has_value_ = true;
            that.reset();
        }
    }

    void reset() noexcept {
        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
        new (&value_) T(std::forward<Args>(args)...);
        has_value_ = true;
    }

    template <typename U, typename... Args>
    void emplace(std::initializer_list<U> ilist, Args&&... args) {
        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
        new (&value_) T(ilist, std::forward<Args>(args)...);
        has_value_ = true;
    }

#pragma endregion

private:
    bool has_value_;
    union {
        T value_;
        Nullopt nullopt_;
    };
};

// Comparisons between optional values.
template <typename T, typename U>
constexpr bool operator==(const Optional<T>& lhs, const Optional<U>& rhs) {
    return static_cast<bool>(lhs) == static_cast<bool>(rhs) && (!lhs || *lhs == *rhs);
}

template <typename T, typename U>
constexpr bool operator!=(const Optional<T>& lhs, const Optional<U>& rhs) {
    return static_cast<bool>(lhs) != static_cast<bool>(rhs) || (static_cast<bool>(lhs) && *lhs != *rhs);
}

template <typename T, typename U>
constexpr bool operator<(const Optional<T>& lhs, const Optional<U>& rhs) {
    return static_cast<bool>(rhs) && (!lhs || *lhs < *rhs);
}

template <typename T, typename U>
constexpr bool operator>(const Optional<T>& lhs, const Optional<U>& rhs) {
    return static_cast<bool>(lhs) && (!rhs || *lhs > *rhs);
}

template <typename T, typename U>
constexpr bool operator<=(const Optional<T>& lhs, const Optional<U>& rhs) {
    return !lhs || (static_cast<bool>(rhs) && *lhs <= *rhs);
}

template <typename T, typename U>
constexpr bool operator>=(const Optional<T>& lhs, const Optional<U>& rhs) {
    return !rhs || (static_cast<bool>(lhs) && *lhs >= *rhs);
}

// Comparisons with nullopt.
template <typename T>
constexpr bool operator==(const Optional<T>& lhs, Nullopt) noexcept {
    return !lhs;
}

#ifdef __cpp_lib_three_way_comparison
template <typename T>
constexpr std::strong_ordering operator<=>(const Optional<T>& lhs, Nullopt) noexcept {
    return static_cast<bool>(lhs) <=> false;
}
#else
// TODO: add comprisons
#endif

#if __cpp_deduction_guides >= 201606
// CTAD
template <typename T>
Optional(T) -> Optional<T>;
#endif

template <typename T>
void swap(Optional<T>& lhs, Optional<T>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename T>
Optional<T> makeOptional(T value) {
    return Optional<T>(std::move(value));
}

}  // namespace pycstl
