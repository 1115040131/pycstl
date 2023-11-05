#pragma once

#include <format>

namespace pycstl {

template <typename T, typename Allocator = std::allocator<T>>
class Vector {
public:
    using value_type = T;
    using allocator_type = Allocator;

    using iterator = T*;
    using const_iterator = const T*;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // 构造函数 (1)
    constexpr Vector() noexcept(noexcept(Allocator())) {
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
        alloc_ = Allocator();
    }

    // 构造函数 (2)
    constexpr explicit Vector(const Allocator& alloc) noexcept : alloc_(alloc) {}

    // 构造函数 (3)
    constexpr Vector(size_t count, const T& value, const Allocator& alloc = Allocator()) : alloc_(alloc) {
        data_ = alloc_.allocate(count);
        for (size_t i = 0; i < count; i++) {
            std::construct_at(&data_[i], value);
        }
        size_ = count;
        capacity_ = count;
    }

    // 构造函数 (4)
    constexpr explicit Vector(size_t count, const Allocator& alloc = Allocator()) : alloc_(alloc) {
        data_ = alloc_.allocate(count);
        for (size_t i = 0; i < count; i++) {
            std::construct_at(&data_[i]);
        }
        size_ = count;
        capacity_ = count;
    }

    // 构造函数 (5)
    template <std::input_iterator InpuIt>
    constexpr Vector(InpuIt first, InpuIt last, const Allocator& alloc = Allocator()) : alloc_(alloc) {
        size_t count = last - first;
        data_ = alloc_.allocate(count);
        for (size_t i = 0; i < count; i++) {
            std::construct_at(&data_[i], *first);
            ++first;
        }
        size_ = count;
        capacity_ = count;
    }

    // 构造函数 (6)
    constexpr Vector(const Vector& other) {
        size_ = other.size_;
        capacity_ = other.size_;
        alloc_ = other.alloc_;
        if (size_ != 0) {
            data_ = alloc_.allocate(size_);
            for (size_t i = 0; i < size_; i++) {
                std::construct_at(&data_[i], std::as_const(other.data_[i]));
            }
        } else {
            data_ = nullptr;
        }
    }

    // 构造函数 (8)
    constexpr Vector(Vector&& other) noexcept {
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.size_;
        alloc_ = other.alloc_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // 构造函数 (10)
    constexpr Vector(std::initializer_list<T> init, const Allocator& alloc = Allocator())
        : Vector(init.begin(), init.end(), alloc) {}

    Vector& operator=(const Vector& other) {
        if (&other == this) [[unlikely]] {
            return *this;
        }

        reserve(other.size_);
        size_ = other.size_;
        capacity_ = other.size_;
        for (size_t i = 0; i < size_; i++) {
            std::construct_at(&data_[i], std::as_const(other.data_[i]));
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (&other == this) [[unlikely]] {
            return *this;
        }

        if (capacity_ != 0) {
            alloc_.deallocate(data_, capacity_);
        }
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    Vector& operator=(std::initializer_list<T> init) {
        auto first = init.begin();
        auto last = init.end();
        size_t count = last - first;
        data_ = alloc_.allocate(count);
        for (size_t i = 0; i < count; i++) {
            data_[i] = *first;
            ++first;
        }
        size_ = count;
        capacity_ = count;
        return *this;
    }

    ~Vector() noexcept {
        if (capacity_) {
            alloc_.deallocate(data_, capacity_);
        }
    }

    void assign(size_t count, const T& value) {
        clear();
        insert(begin(), count, value);
    }

    template <std::input_iterator InpuIt>
    void assign(InpuIt first, InpuIt last) {
        clear();
        insert(begin(), first, last);
    }

    void assign(std::initializer_list<T> init) {
        assign(init.begin(), init.end());
    }

    allocator_type get_allocator() const noexcept {
        return alloc_;
    }

    // 元素访问

    T& at(size_t pos) {
        if (pos >= size_) [[unlikely]] {
            _throw_out_of_range(pos);
        }
        return data_[pos];
    }

    const T& at(size_t pos) const {
        if (pos >= size_) [[unlikely]] {
            _throw_out_of_range(pos);
        }
        return data_[pos];
    }

    T& operator[](size_t pos) noexcept { return data_[pos]; }

    const T& operator[](size_t pos) const noexcept { return data_[pos]; }

    T& front() noexcept { return data_[0]; }

    const T& front() const noexcept { return data_[0]; }

    T& back() noexcept { return data_[size_ - 1]; }

    const T& back() const noexcept { return data_[size_ - 1]; }

    T* data() noexcept { return data_; }

    const T* data() const noexcept { return data_; }

    // 迭代器

    iterator begin() noexcept {
        return data_;
    }

    const_iterator begin() const noexcept {
        return data_;
    }

    const_iterator cbegin() const noexcept {
        return data_;
    }

    iterator end() noexcept {
        return data_ + size_;
    }

    const_iterator end() const noexcept {
        return data_ + size_;
    }

    const_iterator cend() const noexcept {
        return data_ + size_;
    }

    reverse_iterator rbegin() noexcept {
        return std::make_reverse_iterator(data_);
    }

    const_reverse_iterator rbegin() const noexcept {
        return std::make_reverse_iterator(data_);
    }

    const_reverse_iterator crbegin() const noexcept {
        return std::make_reverse_iterator(data_);
    }

    reverse_iterator rend() noexcept {
        return std::make_reverse_iterator(data_ + size_);
    }

    const_reverse_iterator rend() const noexcept {
        return std::make_reverse_iterator(data_ + size_);
    }

    const_reverse_iterator crend() const noexcept {
        return std::make_reverse_iterator(data_ + size_);
    }

    // 容量

    bool empty() const noexcept { return size_ > 0; }

    size_t size() const noexcept { return size_; }

    void reserve(size_t count) {
        if (count <= capacity_) [[likely]] {
            return;
        }
#ifdef _MSC_VER
        count = std::max(count, static_cast<size_t>(capacity_ * 1.5));
#else
        count = std::max(count, capacity_ * 2);
#endif
        auto old_data = data_;
        auto old_capacity = capacity_;
        if (count == 0) {
            data_ = nullptr;
            capacity_ = 0;
        } else {
            data_ = alloc_.allocate(count);
            capacity_ = count;
        }
        if (old_capacity) {
            for (size_t i = 0; i < size_; i++) {
                std::construct_at(&data_[i], std::as_const(old_data[i]));
            }
            alloc_.deallocate(old_data, old_capacity);
        }
    }

    size_t capacity() const noexcept { return capacity_; }

    void shrink_to_fit() {
        if (capacity_ == size_) [[unlikely]] {
            return;
        }

        auto old_data = data_;
        auto old_capacity = capacity_;
        capacity_ = size_;
        if (size_ == 0) {
            data_ = nullptr;
        } else {
            data_ = alloc_.allocate(size_);
        }
        if (old_capacity) {
            for (size_t i = 0; i < size_; i++) {
                std::construct_at(&data_[i], std::as_const(old_data[i]));
            }
            alloc_.deallocate(old_data, old_capacity);
        }
    }

    // 修改器

    void clear() noexcept {
        for (size_t i = 0; i < size_; i++) {
            std::destroy_at(&data_[i]);
        }
        size_ = 0;
    }

    iterator insert(const_iterator pos, const T& value) {
        size_t insert_idx = pos - data_;
        reserve(size_ + 1);
        for (size_t offset = size_; offset > insert_idx; offset--) {
            std::construct_at(&data_[offset], std::move_if_noexcept(data_[offset - 1]));
            std::destroy_at(&data_[offset - 1]);
        }
        std::construct_at(&data_[insert_idx], value);
        size_++;
        return data_ + insert_idx;
    }

    iterator insert(const_iterator pos, size_t count, const T& value) {
        if (count == 0) [[unlikely]] {
            return const_cast<iterator>(pos);
        }
        size_t insert_idx = pos - data_;
        reserve(size_ + count);
        for (size_t offset = size_; offset > insert_idx; offset--) {
            std::construct_at(&data_[offset - 1 + count], std::move_if_noexcept(data_[offset - 1]));
            std::destroy_at(&data_[offset - 1]);
        }
        for (size_t offset = insert_idx; offset < insert_idx + count; offset++) {
            std::construct_at(&data_[offset], value);
        }
        size_ += count;
        return data_ + insert_idx;
    }

    template <std::input_iterator InpuIt>
    iterator insert(const_iterator pos, InpuIt first, InpuIt last) {
        size_t count = last - first;
        if (count == 0) [[unlikely]] {
            return const_cast<iterator>(pos);
        }
        size_t insert_idx = pos - data_;
        reserve(size_ + count);
        for (size_t offset = size_; offset > insert_idx; offset--) {
            std::construct_at(&data_[offset - 1 + count], std::move_if_noexcept(data_[offset - 1]));
            std::destroy_at(&data_[offset - 1]);
        }
        for (size_t offset = insert_idx; offset < insert_idx + count; offset++) {
            std::construct_at(&data_[offset], *first);
            ++first;
        }
        size_ += count;
        return data_ + insert_idx;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> init) {
        return insert(pos, init.begin(), init.end());
    }

    template <typename... Args>
    T& emplace(const_iterator pos, Args&&... args) {
        size_t insert_idx = pos - data_;
        reserve(size_ + 1);
        for (size_t offset = size_; offset > insert_idx; offset--) {
            std::construct_at(&data_[offset], std::move_if_noexcept(data_[offset - 1]));
            std::destroy_at(&data_[offset - 1]);
        }
        std::construct_at(&data_[insert_idx], std::forward<Args>(args)...);
        size_++;
        return data_ + insert_idx;
    }

    void resize(size_t count, const T& value = T()) {
        reserve(count);
        for (size_t i = size_; i < count; i++) {
            std::construct_at(&data_[i], value);
        }
        size_ = count;
    }

    iterator erase(const_iterator pos) noexcept {
        for (size_t i = pos - data_ + 1; i < size_; i++) {
            data_[i - 1] = std::move_if_noexcept(data_[i]);
        }
        size_--;
        std::destroy_at(&data_[size_]);
        return const_cast<iterator>(pos);
    }

    iterator erase(const_iterator first, const_iterator last) noexcept {
        size_t diff = last - first;
        for (size_t i = last - data_; i < size_; i++) {
            data_[i - diff] = std::move_if_noexcept(data_[i]);
        }
        size_ -= diff;
        for (size_t i = size_; i < size_ + diff; i++) {
            std::destroy_at(&data_[i]);
        }
        return const_cast<iterator>(first);
    }

    void push_back(const T& value) {
        reserve(size_ + 1);
        std::construct_at(&data_[size_], value);
        size_++;
    }

    void push_back(T&& value) {
        reserve(size_ + 1);
        std::construct_at(&data_[size_], std::move_if_noexcept(value));
        size_++;
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        reserve(size_ + 1);
        std::construct_at(&data_[size_], std::forward<Args>(args)...);
        size_++;
        return back();
    }

    void swap(Vector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    void _throw_out_of_range(size_t pos) {
        throw std::out_of_range(std::format("out of range! vector<{}> size={} visit ", typeid(T).name(), size_, pos));
    }

private:
    T* data_;
    size_t size_;
    size_t capacity_;
#ifdef _MSC_VER
    [[msvc::no_unique_address]] Allocator alloc_;
#else
    [[no_unique_address]] Allocator alloc_;
#endif
};

}  // namespace pycstl
