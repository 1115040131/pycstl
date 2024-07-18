#include <string>

#include <fmt/core.h>
#include <gtest/gtest.h>

namespace pyc {

using namespace std::string_literals;

template <typename T>
const char* f(const T a) {
    return typeid(a).name();
}

TEST(TemplateArgumentDeductionTest, TemplateArgumentDeductionTest) {
    fmt::println("T is {}", f(47));
    fmt::println("T is {}", f(47L));
    fmt::println("T is {}", f(47.0));
    fmt::println("T is {}", f("47"));
    // fmt::println("T is {}", f("47"_s));
}

template <typename T>
class Sum {
    T v{};

public:
    // (values+...) 可将操作符应用于一个参数包的所有成员
    template <typename... Ts>
    Sum(Ts&&... values) : v{(values + ...)} {}
    const T& value() const { return v; }
};

// 模板实参推导不能为所有这些不同的形参找到一个公共类型，编译器将会报出一个错误消息, 使用推导指引
template <typename... Ts>
Sum(Ts&&...) -> Sum<std::common_type_t<Ts...>>;

TEST(TemplateArgumentDeductionTest, DeducationGuidesTest) {
    Sum s1{1u, 2.0, 3, 4.0f};
    EXPECT_EQ(typeid(s1.value()).name(), typeid(double).name());
    EXPECT_EQ(s1.value(), 10);

    Sum s2{"abc"s, "def"};
    EXPECT_EQ(typeid(s2.value()).name(), typeid(std::string).name());
    EXPECT_EQ(s2.value(), "abcdef");
}

}  // namespace pyc
