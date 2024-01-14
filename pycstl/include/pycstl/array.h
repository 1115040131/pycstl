#pragma once

#include <fmt/format.h>

namespace pycstl {

template <typename _Tp, size_t _N>
class Array {
public:
    using value_type = _Tp;
    using pointer = _Tp*;
    using const_pointer = const _Tp*;
    using reference = _Tp&;
    using const_reference = const _Tp&;

    using iterator = _Tp*;
    using const_iterator = const _Tp*;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_t Size() noexcept { return _N; }

    // 元素访问

    _Tp& at(size_t _Pos) {
        if (_Pos >= _N) [[unlikely]] {
            throwOutOfRange(_Pos);
        }
        return _M_elements[_Pos];
    }

    const _Tp& at(size_t _Pos) const {
        if (_Pos >= _N) [[unlikely]] {
            throwOutOfRange(_Pos);
        }
        return _M_elements[_Pos];
    }

    _Tp& operator[](size_t _Pos) noexcept { return _M_elements[_Pos]; }

    const _Tp& operator[](size_t _Pos) const noexcept { return _M_elements[_Pos]; }

    _Tp& front() noexcept { return _M_elements[0]; }

    const _Tp& front() const noexcept { return _M_elements[0]; }

    _Tp& back() noexcept { return _M_elements[_N - 1]; }

    const _Tp& back() const noexcept { return _M_elements[_N - 1]; }

    _Tp* data() noexcept { return _M_elements; }

    const _Tp* data() const noexcept { return _M_elements; }

    const _Tp* cdata() const noexcept { return _M_elements; }

    // 迭代器

    iterator begin() noexcept { return _M_elements; }

    const_iterator begin() const noexcept { return _M_elements; }

    const_iterator cbegin() const noexcept { return _M_elements; }

    iterator end() noexcept { return _M_elements + _N; }

    const_iterator end() const noexcept { return _M_elements + _N; }

    const_iterator cend() const noexcept { return _M_elements + _N; }

    // 容量

    constexpr bool empty() const noexcept { return false; }

    constexpr size_t size() const noexcept { return _N; }

    constexpr size_t max_size() const noexcept { return _N; }

    // 操作

    void fill(const _Tp& _Val) noexcept(std::is_nothrow_copy_assignable_v<_Tp>) {
        for (size_t _Pos = 0; _Pos < _N; _Pos++) {
            _M_elements[_Pos] = _Val;
        }
    }

    void swap(Array& _Other) noexcept(std::is_nothrow_swappable_v<_Tp>) {
        for (size_t _Pos = 0; _Pos < _N; _Pos++) {
            std::swap(_M_elements[_Pos], _Other._M_elements[_Pos]);
        }
    }

public:
    _Tp _M_elements[_N];

private:
    void throwOutOfRange(size_t _Pos) {
        throw std::out_of_range(fmt::format("out of range! array<{}, {}>.at({}) ", typeid(_Tp).name(), _N, _Pos));
    }
};

#ifdef _MSC_VER
#define _LIBPYCCXX_UNREACHABLE() __assume(0)
#elif defined(__GNUC__)
#define _LIBPYCCXX_UNREACHABLE() __builtin_unreachable()
#elif defined(__clang__)
#define _LIBPYCCXX_UNREACHABLE() __builtin_unreachable()
#else
#define _LIBPYCCXX_UNREACHABLE() \
    do {                         \
    } while (1)
#endif

template <typename _Tp>
class Array<_Tp, 0> {
public:
    using value_type = _Tp;
    using pointer = _Tp*;
    using const_pointer = const _Tp*;
    using reference = _Tp&;
    using const_reference = const _Tp&;

    using iterator = _Tp*;
    using const_iterator = const _Tp*;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_t Size() noexcept { return 0; }

    // 元素访问

    _Tp& at(size_t _Pos) { throwOutOfRange(_Pos); }

    const _Tp& at(size_t _Pos) const { throwOutOfRange(_Pos); }

    _Tp& operator[](size_t _Pos) noexcept { _LIBPYCCXX_UNREACHABLE(); }

    const _Tp& operator[](size_t _Pos) const noexcept { _LIBPYCCXX_UNREACHABLE(); }

    _Tp& front() noexcept { _LIBPYCCXX_UNREACHABLE(); }

    const _Tp& front() const noexcept { _LIBPYCCXX_UNREACHABLE(); }

    _Tp& back() noexcept { _LIBPYCCXX_UNREACHABLE(); }

    const _Tp& back() const noexcept { _LIBPYCCXX_UNREACHABLE(); }

    _Tp* data() noexcept { return nullptr; }

    const _Tp* data() const noexcept { return nullptr; }

    const _Tp* cdata() const noexcept { return nullptr; }

    // 迭代器

    iterator begin() noexcept { return nullptr; }

    const_iterator begin() const noexcept { return nullptr; }

    const_iterator cbegin() const noexcept { return nullptr; }

    iterator end() noexcept { return nullptr; }

    const_iterator end() const noexcept { return nullptr; }

    const_iterator cend() const noexcept { return nullptr; }

    // 容量

    constexpr bool empty() const noexcept { return true; }

    constexpr size_t size() const noexcept { return 0; }

    constexpr size_t max_size() const noexcept { return 0; }

    // 操作

    void fill(const _Tp& _Val) noexcept {}

    void swap(Array& _Other) noexcept {}

private:
    void throwOutOfRange(size_t _Pos) {
        throw std::out_of_range(fmt::format("out of range! array<{}, {}>.at({}) ", typeid(_Tp).name(), 0, _Pos));
    }
};

// 推导指引
template <typename _Tp, typename... _Ts>
Array(_Tp, _Ts...) -> Array<_Tp, 1 + sizeof...(_Ts)>;

template <typename _Tp, size_t _N, size_t... _Idx>
constexpr Array<std::decay_t<_Tp>, _N> _To_Array_Impl(_Tp (&_Array)[_N], std::index_sequence<_Idx...>) {
    return {{_Array[_Idx]...}};
}

template <typename _Tp, size_t _N>
constexpr Array<std::decay_t<_Tp>, _N> to_array(_Tp (&_Array)[_N]) {
    return _To_Array_Impl(_Array, std::make_index_sequence<_N>{});
}

}  // namespace pycstl
