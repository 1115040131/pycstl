#include <span>

#include <fmt/base.h>
#include <gtest/gtest.h>

namespace pyc {

template <typename T>
void PrintSpan(std::span<T> s) {
    fmt::println("size: {}", s.size());
    fmt::println("size_bytes: {}", s.size_bytes());
    for (const auto& v : s) {
        fmt::print("{} ", v);
    }
    fmt::print("\n");
}

TEST(SpanTest, SpanTest) {
    int c_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    PrintSpan<int>(c_array);
}

// span 定义如下, Extent 是一个 constexpr size_t 常量, 其要么是底层数据中的元素数量, 要么是 std::dynamic_extent,
// 这表明其大小可变, 这允许 span 使用底层结构, 但大小并不总是相同
template <typename T, size_t Extent = std::dynamic_extent>
class span {
    T* data_;
    size_t count;

public:
    //...
};

}  // namespace pyc
