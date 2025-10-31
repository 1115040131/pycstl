#pragma once

#include <exception>

namespace pycstl {

template <size_t I>
struct InPlaceIndex {
    explicit InPlaceIndex() = default;
};

template <size_t I>
constexpr InPlaceIndex<I> in_place_index{};

class BadVariantAccess : public std::exception {
public:
    BadVariantAccess() = default;
    virtual ~BadVariantAccess() = default;

    virtual const char* what() const noexcept override { return "BadVariantAccess"; }
};

template <typename V, typename T>
struct VariantIndex;  // typename -> size_t

template <typename V, size_t I>
struct VariantAlternative;  // size_t -> typename

template <typename... Ts>
class Variant {
public:
    template <typename T>
        requires(std::is_same_v<T, Ts> || ...)
    Variant(T value) : index_(VariantIndex<Variant, T>::value) {
        new (union_) T(value);
    }

    Variant(const Variant& that) {
        index_ = that.index_;
        copy_constructors_[index_](union_, that.union_);
    }

    Variant(Variant&& that) {
        index_ = that.index_;
        move_constructors_[index_](union_, that.union_);
    }

    template <size_t I, typename... Args>
    explicit Variant(InPlaceIndex<I>, Args&&... args) : index_(I) {
        new (union_) typename VariantAlternative<Variant, I>::type(std::forward<Args>(args)...);
    }

    Variant& operator=(const Variant& that) {
        destructors_[index_](union_);
        index_ = that.index_;
        copy_assignments_[index_](union_, that.union_);
        return *this;
    }

    Variant& operator=(Variant&& that) {
        destructors_[index_](union_);
        index_ = that.index_;
        move_assignments_[index_](union_, that.union_);
        return *this;
    }

    ~Variant() noexcept { destructors_[index_](union_); }

#pragma region 观察者

    constexpr size_t index() const noexcept { return index_; }

#pragma endregion

#pragma region 访问

    template <typename Visitor>
    auto visit(Visitor&& visitor) {
        return visitors_table_<Visitor>[index_](union_, std::forward<Visitor>(visitor));
    }

    template <typename Visitor>
    auto visit(Visitor&& visitor) const {
        return const_visitors_table_<Visitor>[index_](union_, std::forward<Visitor>(visitor));
    }

#pragma endregion

    template <typename T>
    constexpr bool holds_alternative() const noexcept {
        return VariantIndex<Variant, T>::value == index_;
    }

    template <size_t I>
    typename VariantAlternative<Variant, I>::type& get() {
        static_assert(I < sizeof...(Ts), "I out of range!");

        if (index_ != I) {
            throw BadVariantAccess();
        }

        using MemberType = typename VariantAlternative<Variant, I>::type;
        return *reinterpret_cast<MemberType*>(union_);
    }

    template <typename T>
    T& get() {
        return get<VariantIndex<Variant, T>::value>();
    }

    template <size_t I>
    const typename VariantAlternative<Variant, I>::type& get() const {
        static_assert(I < sizeof...(Ts), "I out of range!");

        if (index_ != I) {
            throw BadVariantAccess();
        }

        using MemberType = typename VariantAlternative<Variant, I>::type;
        return *reinterpret_cast<const MemberType*>(union_);
    }

    template <typename T>
    const T& get() const {
        return get<VariantIndex<Variant, T>::value>();
    }

    template <size_t I>
    typename VariantAlternative<Variant, I>::type* get_if() {
        static_assert(I < sizeof...(Ts), "I out of range!");

        if (index_ != I) {
            return nullptr;
        }

        using MemberType = typename VariantAlternative<Variant, I>::type;
        return reinterpret_cast<MemberType*>(union_);
    }

    template <typename T>
    T* get_if() {
        return get_if<VariantIndex<Variant, T>::value>();
    }

    template <size_t I>
    const typename VariantAlternative<Variant, I>::type* get_if() const {
        static_assert(I < sizeof...(Ts), "I out of range!");

        if (index_ != I) {
            return nullptr;
        }

        using MemberType = typename VariantAlternative<Variant, I>::type;
        return reinterpret_cast<const MemberType*>(union_);
    }

    template <typename T>
    const T* get_if() const {
        return get_if<VariantIndex<Variant, T>::value>();
    }

private:
    using CopyConstructorFunction = void (*)(char*, const char*) noexcept;
    inline static CopyConstructorFunction copy_constructors_[sizeof...(Ts)] = {
        [](char* union_dst, const char* union_src) noexcept {
            new (union_dst) Ts(*reinterpret_cast<const Ts*>(union_src));
        }...};

    using CopyAssignmentFunction = void (*)(char*, const char*) noexcept;
    inline static CopyAssignmentFunction copy_assignments_[sizeof...(Ts)] = {
        [](char* union_dst, const char* union_src) noexcept {
            new (union_dst) Ts(*reinterpret_cast<const Ts*>(union_src));
        }...};

    using MoveConstructorFunction = void (*)(char*, char*) noexcept;
    inline static MoveConstructorFunction move_constructors_[sizeof...(Ts)] = {
        [](char* union_dst, char* union_src) noexcept {
            new (union_dst) Ts(std::move(*reinterpret_cast<Ts*>(union_src)));
        }...};

    using MoveAssignmentFunction = void (*)(char*, char*) noexcept;
    inline static MoveAssignmentFunction move_assignments_[sizeof...(Ts)] = {
        [](char* union_dst, char* union_src) noexcept {
            new (union_dst) Ts(std::move(*reinterpret_cast<Ts*>(union_src)));
        }...};

    using DestructorFunction = void (*)(char*) noexcept;
    inline static DestructorFunction destructors_[sizeof...(Ts)] = {
        [](char* union_ptr) noexcept { reinterpret_cast<Ts*>(union_ptr)->~Ts(); }...};

    template <typename Visitor>
    using VisitorFunction = std::common_type_t<typename std::invoke_result_t<Visitor, Ts&>...> (*)(char*,
                                                                                                   Visitor&&);

    template <typename Visitor>
    inline static VisitorFunction<Visitor> visitors_table_[sizeof...(Ts)] = {
        [](char* union_ptr, Visitor&& visitor) -> auto {
            return std::forward<Visitor>(visitor)(*reinterpret_cast<const Ts*>(union_ptr));
        }...};

    template <typename Visitor>
    using ConstVisitorFunction =
        std::common_type_t<typename std::invoke_result_t<Visitor, Ts&>...> (*)(const char*, Visitor&&);

    template <typename Visitor>
    inline static ConstVisitorFunction<Visitor> const_visitors_table_[sizeof...(Ts)] = {
        [](const char* union_ptr, Visitor&& visitor) -> auto {
            return std::forward<Visitor>(visitor)(*reinterpret_cast<const Ts*>(union_ptr));
        }...};

private:
    int index_;

    char union_[std::max({sizeof(Ts)...})];
};

template <typename T, typename... Ts>
struct VariantIndex<Variant<T, Ts...>, T> {
    static constexpr size_t value = 0;
};

template <typename T0, typename T, typename... Ts>
struct VariantIndex<Variant<T0, Ts...>, T> {
    static constexpr size_t value = VariantIndex<Variant<Ts...>, T>::value + 1;
};

template <typename T, typename... Ts>
struct VariantAlternative<Variant<T, Ts...>, 0> {
    using type = T;
};

template <typename T, typename... Ts, size_t I>
struct VariantAlternative<Variant<T, Ts...>, I> {
    using type = VariantAlternative<Variant<Ts...>, I - 1>::type;
};

}  // namespace pycstl