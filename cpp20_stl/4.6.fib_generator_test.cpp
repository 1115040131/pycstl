#include <ranges>

#include <gtest/gtest.h>

#include "utils.h"

namespace pyc {

class fib_generator {
public:
    using fib_t = unsigned long;

    // 若想让生成器与算法库一起工作，需要提供特性别名。
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::remove_cv_t<fib_t>;
    using difference_type = std::ptrdiff_t;
    using pointer = const fib_t*;
    using reference = const fib_t&;

    explicit fib_generator(fib_t stop = 0) : stop_{stop} {}

    fib_t operator*() const { return a_; }

    constexpr fib_generator& operator++() {
        do_fib();
        ++count_;
        return *this;
    }

    fib_generator operator++(int) {
        auto tmp = *this;
        do_fib();
        ++count_;
        return tmp;
    }

    bool operator!=(const fib_generator& that) const noexcept { return count_ != that.count_; }
    bool operator==(const fib_generator& that) const noexcept { return !(*this != that); }

    const fib_generator& begin() const { return *this; }

    const fib_generator end() const {
        auto sentinel = fib_generator();
        sentinel.count_ = stop_;
        return sentinel;
    }

    fib_t size() const { return stop_; }

private:
    constexpr void do_fib() {
        const fib_t old_b = b_;
        b_ += a_;
        a_ = old_b;
    }

private:
    fib_t stop_{};    // 要生成的值的数量
    fib_t count_{0};  // 已经生成的值的数量
    fib_t a_{1};
    fib_t b_{1};
};

TEST(FibGeneratorTest, FibGeneratorTest) {
    fib_generator fib{10};
    printc(fib);

    auto x = std::ranges::views::transform(fib, [](unsigned long x) { return x * x; });
    printc(x, "squared: ");
}

}  // namespace pyc
