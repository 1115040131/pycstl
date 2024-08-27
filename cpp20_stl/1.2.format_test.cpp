#include <version>  // C++20 header that defines feature test macros

#include <gtest/gtest.h>

// 检查是否存在__cpp_lib_format宏，表明<format>支持。
#if defined(__cpp_lib_format)
#include <format>
#else
// 如果不存在，引入fmt库作为替代，并定义 std::format
#include <fmt/format.h>

namespace std {

using fmt::format;
using fmt::format_string;
using fmt::make_format_args;
using fmt::vformat;

}  // namespace std
#endif

#if defined(__cpp_lib_print)
#include <print>
#else
#include <cstdio>

namespace std {

template <typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args) {
    // make_format_args 接受参数包并返回一个对象，该对象包含适合格式化的已擦除类型的值
    string outstr{std::vformat(fmt.get(), std::make_format_args(args...))};
    std::fputs(outstr.c_str(), stdout);
}

template <typename... Args>
void println(std::format_string<Args...> fmt_str, Args&&... args) {
    std::print("{}\n", std::format(fmt_str, std::forward<Args>(args)...));
}

}  // namespace std
#endif

TEST(FormatTest, FormatTest) {
    std::string message = std::format("The answer is {}.", 42);
    EXPECT_EQ(message, "The answer is 42.");
}

TEST(FormatTest, PrintTest) {
    std::string who{"everyone"};
    int ival{42};
    double pi{std::numbers::pi};

    std::print("Hello, {}!\n", who);
    std::println("Hello, {}!", who);
    std::print("π: {}\n", pi);
    std::print("Hello {1} {0}\n", ival, who);
    std::print("{:.^10}\n", ival);
    std::print("{:.5}\n", pi);
}

// GCC13 中暂不支持 formatter
#if defined(__GNUC__) && (__GNUC__ > 13)

// 自定义结构体的 format
struct Frac {
    long n;
    long d;
};

template<>
struct std::formatter<Frac> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return ctx.begin();
	}
	template<typename FormatContext>
	auto format(const Frac& f, FormatContext& ctx) {
		return std::format_to(ctx.out(), "{0:d}/{1:d}", f.a, f.b);
	}
};

TEST(FormatTest, FormatStruct) {
    Frac f{5, 3};
    EXPECT_EQ(std::format("{}", f), "5/3");
}

#endif
