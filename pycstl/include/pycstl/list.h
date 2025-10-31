#pragma once

#include <memory>

namespace pycstl {

template <typename T, typename Allocator = std::allocator<T>>
class List {
private:
    struct ListBaseNode {
        ListBaseNode* prev_;
        ListBaseNode* next_;

        inline T& value() { return static_cast<ListValueNode&>(*this).value_; }
        inline const T& value() const { return static_cast<const ListValueNode&>(*this).value_; }
    };

    struct ListValueNode : ListBaseNode {
        T value_;
    };

    using ListNode = ListBaseNode;

    // 将 T 类型的分配器 rebind 到 ListValueNode<T> 上
    using AllocNode = std::allocator_traits<Allocator>::template rebind_alloc<ListValueNode>;

    ListNode dummy_;
    std::size_t size_ = 0;

#ifdef _MSC_VER
    [[msvc::no_unique_address]] Allocator alloc_;
#else
    [[no_unique_address]] Allocator alloc_;
#endif

    ListNode* newNode() { return AllocNode{alloc_}.allocate(1); }

    void deleteNode(ListNode* node) noexcept {
        AllocNode{alloc_}.deallocate(static_cast<ListValueNode*>(node), 1);
    }

public:
    using value_type = T;
    using allocator_type = Allocator;

    struct iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

    private:
        ListNode* curr;

        explicit iterator(ListNode* node) noexcept : curr(node) {}

        friend class List;
        friend struct const_iterator;

    public:
        iterator() = default;

        iterator& operator++() noexcept {  //++iterator
            curr = curr->next_;
            return *this;
        }

        iterator operator++(int) noexcept {  // iterator++
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator--() noexcept {  //--iterator
            curr = curr->prev_;
            return *this;
        }

        iterator operator--(int) noexcept {  // iterator--
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        T& operator*() const noexcept { return curr->value(); }

        bool operator!=(const iterator& that) const noexcept { return curr != that.curr; }

        bool operator==(const iterator& that) const noexcept { return !(*this != that); }
    };

    struct const_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

    private:
        const ListNode* curr;

        explicit const_iterator(const ListNode* node) noexcept : curr(node) {}

        friend class List;

    public:
        const_iterator() = default;

        const_iterator(iterator that) noexcept : curr(that.curr) {}

        explicit operator iterator() const noexcept { return iterator{const_cast<ListNode*>(curr)}; }

        const_iterator& operator++() noexcept {  //++iterator
            curr = curr->next_;
            return *this;
        }

        const_iterator operator++(int) noexcept {  // iterator++
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        const_iterator& operator--() noexcept {  //--iterator
            curr = curr->prev_;
            return *this;
        }

        const_iterator operator--(int) noexcept {  // iterator--
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        const T& operator*() const noexcept { return curr->value(); }

        bool operator!=(const const_iterator& that) const noexcept { return curr != that.curr; }

        bool operator==(const const_iterator& that) const noexcept { return !(*this != that); }
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    void _uninit_assign(std::size_t count, const T& value) {
        ListNode* prev = &dummy_;
        for (std::size_t i = 0; i < count; ++i) {
            ListNode* curr = newNode();
            prev->next_ = curr;
            curr->prev_ = prev;
            std::construct_at(&curr->value(), value);
            prev = curr;
        }
        dummy_.prev_ = prev;
        prev->next_ = &dummy_;
        size_ = count;
    }

    void _uninit_assign(std::size_t count) {
        ListNode* prev = &dummy_;
        for (std::size_t i = 0; i < count; ++i) {
            ListNode* curr = newNode();
            prev->next_ = curr;
            curr->prev_ = prev;
            std::construct_at(&curr->value());
            prev = curr;
        }
        dummy_.prev_ = prev;
        prev->next_ = &dummy_;
        size_ = count;
    }

    template <std::input_iterator InputIt>
    void _uninit_assign(InputIt first, InputIt last) {
        size_ = 0;
        ListNode* prev = &dummy_;
        while (first != last) {
            ListNode* curr = newNode();
            prev->next_ = curr;
            curr->prev_ = prev;
            std::construct_at(&curr->value(), *first);
            prev = curr;
            ++first;
            ++size_;
        }
        dummy_.prev_ = prev;
        prev->next_ = &dummy_;
    }

    void _uninit_assign(std::initializer_list<T> init) { _uninit_assign(init.begin(), init.end()); }

    void _uninit_move_assign(List&& other) noexcept {
        auto prev = other.dummy_.prev_;
        auto next = other.dummy_.next_;
        dummy_ = other.dummy_;
        prev->next_ = next->prev_ = &dummy_;
        other.dummy_.next_ = other.dummy_.prev_ = &other.dummy_;
        size_ = other.size_;
        other.size_ = 0;
    }

public:
    // 构造函数 (1)
    List() noexcept { _uninit_assign(0); }

    // 构造函数 (2)
    explicit List(const Allocator& alloc) noexcept : alloc_(alloc) { _uninit_assign(0); }

    // 构造函数 (3)
    List(std::size_t count, const T& value, const Allocator& alloc = Allocator()) : alloc_(alloc) {
        _uninit_assign(count, value);
    }

    // 构造函数 (4)
    explicit List(std::size_t count, const Allocator& alloc = Allocator()) : alloc_(alloc) {
        _uninit_assign(count);
    }

    // input_iterator 支持 *it it++ ++it it!=it it==it
    // output_iterator 支持 *it=val it++ ++it it!=it it==it
    // forward_iterator 支持 *it *it=val it++ ++it it!=it it==it
    // bidirectional_iterator 支持 *it *it=val it++ ++it it-- --it it!=it it==it
    // random_access_iterator 支持 *it *it=val it[n] it[n]=val it++ ++it it-- --it
    //                             it+=n it-=n it+n n+it it-n it-it it!=it it==it

    // 构造函数 (5)
    template <std::input_iterator InputIt>
    List(InputIt first, InputIt last, const Allocator& alloc = Allocator()) : alloc_(alloc) {
        _uninit_assign(first, last);
    }

    // 构造函数 (6) 拷贝构造函数
    List(const List& other) : alloc_(other.alloc_) { _uninit_assign(other.begin(), other.end()); }

    // 构造函数 (7)
    List(const List& other, const Allocator& alloc) : alloc_(alloc) { _uninit_assign(other.begin(), other.end()); }

    // 构造函数 (8) 移动构造函数
    List(List&& other) noexcept : alloc_(std::move(other.alloc_)) { _uninit_move_assign(std::move(other)); }

    // 构造函数 (9)
    List(List&& other, const Allocator& alloc) noexcept : alloc_(alloc) { _uninit_move_assign(std::move(other)); }

    // 构造函数(10)
    List(std::initializer_list<T> init, const Allocator& alloc = Allocator()) : alloc_(alloc) {
        _uninit_assign(init.begin(), init.end());
    }

    // 析构函数
    ~List() noexcept { clear(); }

    // 拷贝赋值运算符
    List& operator=(const List& other) {
        if (this != &other) {
            clear();
            _uninit_assign(other.begin(), other.end());
            alloc_ = other.alloc_;
        }
        return *this;
    }

    // 移动赋值运算符
    List& operator=(List&& other) noexcept {
        if (this != &other) {
            clear();
            _uninit_move_assign(std::move(other));
            alloc_ = std::move(other.alloc_);
        }
        return *this;
    }

    List& operator=(std::initializer_list<T> init) {
        clear();
        _uninit_assign(init.begin(), init.end());
        return *this;
    }

    void assign(std::size_t count, const T& value) {
        clear();
        _uninit_assign(count, value);
    }

    template <std::input_iterator InputIt>
    void assign(InputIt first, InputIt last) {
        clear();
        _uninit_assign(first, last);
    }

    void assign(std::initializer_list<T> init) {
        clear();
        _uninit_assign(init.begin(), init.end());
    }

    Allocator get_allocator() const noexcept { return alloc_; }

#pragma region 元素访问

    T& front() noexcept { return dummy_.next_->value(); }

    const T& front() const noexcept { return dummy_.next_->value(); }

    T& back() noexcept { return dummy_.prev_->value(); }
    const T& back() const noexcept { return dummy_.prev_->value(); }

#pragma endregion

#pragma region 迭代器

    iterator begin() noexcept { return iterator{dummy_.next_}; }

    iterator end() noexcept { return iterator{&dummy_}; }

    const_iterator cbegin() const noexcept { return const_iterator{dummy_.next_}; }

    const_iterator cend() const noexcept { return const_iterator{&dummy_}; }

    const_iterator begin() const noexcept { return cbegin(); }

    const_iterator end() const noexcept { return cend(); }

    reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }

    reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const noexcept { return std::make_reverse_iterator(cend()); }

    const_reverse_iterator crend() const noexcept { return std::make_reverse_iterator(cbegin()); }

    const_reverse_iterator rbegin() const noexcept { return crbegin(); }

    const_reverse_iterator rend() const noexcept { return crend(); }

#pragma endregion

#pragma region 容量

    bool empty() const noexcept { return dummy_.next_ != &dummy_; }

    std::size_t size() const noexcept { return size_; }

    static constexpr std::size_t max_size() noexcept { return std::numeric_limits<std::size_t>::max(); }

#pragma endregion

#pragma region 修改器

    void clear() noexcept {
        ListNode* curr = dummy_.next_;
        while (curr != &dummy_) {
            std::destroy_at(&curr->value());
            auto next = curr->next_;
            deleteNode(curr);
            curr = next;
        }
        dummy_.next_ = dummy_.prev_ = &dummy_;
        size_ = 0;
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        auto node = const_cast<ListNode*>(pos.curr);  // 插入位置的节点
        ListNode* curr = newNode();                   // 新节点
        curr->prev_ = node->prev_;
        curr->next_ = node;
        std::construct_at(&curr->value(), std::forward<Args>(args)...);
        node->prev_->next_ = curr;
        node->prev_ = curr;
        ++size_;
        return iterator{curr};
    }

    iterator insert(const_iterator pos, const T& value) { return emplace(pos, value); }

    iterator insert(const_iterator pos, T&& value) { return emplace(pos, std::move_if_noexcept(value)); }

    iterator insert(const_iterator pos, std::size_t count, const T& value) {
        if (count == 0) [[unlikely]] {
            return pos;
        }
        auto orig_pos = pos;
        while (count--) {
            pos = emplace(pos, value);
            ++pos;
        }
    }

    template <std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        if (first == last) [[unlikely]] {
            return pos;
        }
        auto orig_pos = pos;
        while (first != last) {
            pos = emplace(pos, *first);
            ++first;
            ++pos;
        }
        return ++orig_pos;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        insert(pos, ilist.begin(), ilist.end());
    }

    iterator erase(const_iterator pos) noexcept {
        auto node = const_cast<ListNode*>(pos.curr);
        auto prev = node->prev_;
        auto next = node->next_;
        prev->next_ = next;
        next->prev_ = prev;
        std::destroy_at(&node->value());
        deleteNode(node);
        --size_;
        return iterator{next};
    }

    iterator erase(const_iterator first, const_iterator last) noexcept {
        while (first != last) {
            first = erase(first);
        }
        return iterator{first};
    }

    void push_back(const T& value) { emplace_back(value); }

    void push_back(T&& value) { emplace_back(std::move_if_noexcept(value)); }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        return *emplace(end(), std::forward<Args>(args)...);
    }

    void pop_back() noexcept { erase(--end()); }

    void push_front(const T& value) { emplace_front(value); }

    void push_front(T&& value) { emplace_front(std::move_if_noexcept(value)); }

    template <typename... Args>
    T& emplace_front(Args&&... args) {
        return *emplace(begin(), std::forward<Args>(args)...);
    }

    void pop_front() noexcept { erase(begin()); }

#pragma endregion

#pragma region 操作

    // 从另一个 list 中移动元素
    void splice(const_iterator pos, List& other) { insert(pos, other.begin(), other.end()); }

    void splice(const_iterator pos, List&& other) {
        inset(pos, std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
    }

    std::size_t remove(const T& value) noexcept {
        std::size_t count = 0;
        auto first = begin();
        auto last = end();
        while (first != last) {
            if (*first == value) {
                first = erase(first);
                ++count;
            } else {
                ++first;
            }
        }
        return count;
    }

    template <typename UnaryPredicate>
    std::size_t remove_if(UnaryPredicate p) {
        std::size_t count = 0;
        auto first = begin();
        auto last = end();
        while (first != last) {
            if (p(*first)) {
                first = erase(first);
                ++count;
            } else {
                ++first;
            }
        }
        return count;
    }

#pragma endregion
};

}  // namespace pycstl