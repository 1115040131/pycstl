#include <fmt/core.h>
#include <gtest/gtest.h>

namespace pyc {

template <typename T>
class ZipIterator {
public:
    using val_t = typename T::value_type;
    using ret_t = std::pair<val_t, val_t>;
    using it_t = typename T::iterator;

    ZipIterator(T& a, T& b)
        : ita_{a.begin()},
          ita_begin_{a.begin()},
          ita_end_{a.end()},
          itb_{b.begin()},
          itb_begin_{b.begin()},
          itb_end_{b.end()} {
    }

    ZipIterator& operator++() {
        ++ita_;
        ++itb_;
        return *this;
    }

    bool operator==(const ZipIterator& that) const { return ita_ == that.ita_ || itb_ == that.itb_; }

    bool operator!=(const ZipIterator& that) const { return !(*this == that); }

    ret_t operator*() const { return {*ita_, *itb_}; }

    ZipIterator begin() const { return ZipIterator(ita_begin_, itb_begin_); }

    ZipIterator end() const { return ZipIterator(ita_end_, itb_end_); }

private:
    ZipIterator(it_t ita, it_t itb) : ita_{ita}, itb_{itb} {}

private:
    it_t ita_{};
    it_t ita_begin_{};
    it_t ita_end_{};
    it_t itb_{};
    it_t itb_begin_{};
    it_t itb_end_{};
};

TEST(ZipIteratorTest, ZipIteratorTest) {
    std::vector<int> a{1, 2, 3, 4, 5};
    std::vector<int> b{6, 7, 8, 9, 10};

    size_t i = 0;
    for (const auto& [x, y] : ZipIterator(a, b)) {
        EXPECT_EQ(x, a[i]);
        EXPECT_EQ(y, b[i]);
        ++i;
    }
}

}  // namespace pyc
