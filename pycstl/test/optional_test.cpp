#include <optional>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <gtest/gtest.h>

#include "pycstl/optional.h"

namespace pycstl {

struct C {
    C(int x, int y) : x_(x), y_(y) {}

    int x_;
    int y_;
};

TEST(OptionalTest, ConstructionTest) {
    {
        Optional<int> opt;
        EXPECT_FALSE(opt);
        EXPECT_FALSE(opt.has_value());
    }
    {
        Optional<int> opt(nullopt);
        EXPECT_FALSE(opt);
        EXPECT_FALSE(opt.has_value());
    }
    {
        Optional<std::string> opt("ss");
        EXPECT_TRUE(opt);
        EXPECT_TRUE(opt.has_value());
        EXPECT_EQ(opt.value(), "ss");
    }
    {
        Optional<std::string> opt1("Copy Construction");
        Optional<std::string> opt2(opt1);
        EXPECT_EQ(opt1.value(), "Copy Construction");
        EXPECT_EQ(opt2.value(), "Copy Construction");
    }
    {
        Optional<std::string> opt1("Move Construction");
        Optional<std::string> opt2(std::move(opt1));
        EXPECT_EQ(opt1.value(), "");
        EXPECT_EQ(opt2.value(), "Move Construction");
    }
    {
        Optional<std::string> opt("Construct From Value");
        EXPECT_TRUE(opt);
        EXPECT_TRUE(opt.has_value());
        EXPECT_EQ(opt.value(), "Construct From Value");

        opt = nullopt;
        EXPECT_FALSE(opt);
        EXPECT_FALSE(opt.has_value());

        opt = "Assignment From Value";
        EXPECT_TRUE(opt);
        EXPECT_TRUE(opt.has_value());
        EXPECT_EQ(opt.value(), "Assignment From Value");

        Optional<std::string> opt1("Copy Assignment");
        opt = opt1;
        EXPECT_EQ(opt.value(), "Copy Assignment");
        EXPECT_EQ(opt1.value(), "Copy Assignment");

        Optional<std::string> opt2("Move Assignment");
        opt = std::move(opt2);
        EXPECT_EQ(opt.value(), "Move Assignment");
        EXPECT_FALSE(opt2);
        EXPECT_FALSE(opt2.has_value());
    }
    { Optional<C> opt; }
    {
        Optional<C> opt(in_place, 1, 2);
        EXPECT_EQ(opt->x_, 1);
        EXPECT_EQ(opt->y_, 2);
    }
    {
        Optional<std::vector<int>> opt{in_place, {1, 2, 3}};
        EXPECT_TRUE(opt);
        EXPECT_EQ(*opt, (std::vector{1, 2, 3}));

        opt.emplace({4, 5, 6, 7});
        EXPECT_EQ(*opt, (std::vector{4, 5, 6, 7}));
    }
}

TEST(OptionalTest, ValueTest) {
    {
        Optional<C> opt;
        EXPECT_FALSE(opt);

        opt.emplace(2, 3);
        EXPECT_TRUE(opt.has_value());
        EXPECT_EQ((*opt).x_, 2);
        EXPECT_EQ(opt->x_, 2);
    }
    {
        Optional<int> opt(nullopt);
        EXPECT_THROW(opt.value(), BadOptionalAccess);
        EXPECT_EQ(opt.value_or(1), 1);
    }
    {
        Optional<std::string> opt("ss");
        EXPECT_EQ(*opt, "ss");
        EXPECT_EQ(opt.value(), "ss");

        auto str = std::move(opt).value();  // 等价于 std::move(opt.value())
        EXPECT_EQ(str, "ss");
        EXPECT_EQ(opt.value(), "");
    }
    {
        Optional<std::string> opt("ss");
        EXPECT_EQ(opt.value_or("aa"), "ss");

        auto str = std::move(opt).value_or("aa");
        EXPECT_EQ(str, "ss");
        EXPECT_EQ(opt.value(), "");
    }
    {
        const Optional<std::string> opt("ss");
        EXPECT_EQ(opt.value(), "ss");

        auto str = std::move(opt).value();
        EXPECT_EQ(str, "ss");
        EXPECT_EQ(opt.value(), "ss");
    }
    {
        const Optional<std::string> opt("ss");
        EXPECT_EQ(opt.value(), "ss");

        auto str = std::move(opt).value_or("aa");
        EXPECT_EQ(str, "ss");
        EXPECT_EQ(opt.value(), "ss");
    }
}

Optional<int> parseInt(std::string s) {
    try {
        return std::stoi(s);
    } catch (...) {
        return nullopt;
    }
}

Optional<int> getInt(std::istream& is) {
    std::string s;
    is >> s;
    if (!is.good()) {
        return nullopt;
    }
    return parseInt(s);
}

TEST(OptionalTest, ParseTest) {
    if (auto opt = parseInt("123")) {
        EXPECT_EQ(*opt, 123);
    } else {
        FAIL() << "Failed to parse int";
    }

    if (auto opt = parseInt("abc")) {
        FAIL() << "Should not parse int";
    }

    // TODO: cin_redirect
    while (auto opt = getInt(std::cin)) {
        fmt::println("{}", *opt);
    }
}

TEST(OptionalTest, ModifyTest) {
    {
        Optional<std::string> opt1;
        Optional<std::string> opt2;

        swap(opt1, opt2);
        EXPECT_FALSE(opt1);
        EXPECT_FALSE(opt2);
    }
    {
        Optional<std::string> opt1("Origin");
        Optional<std::string> opt2;

        swap(opt1, opt2);
        EXPECT_FALSE(opt1);
        EXPECT_TRUE(opt2);
        EXPECT_EQ(*opt2, "Origin");
    }
    {
        Optional<std::string> opt1;
        Optional<std::string> opt2("Origin");

        swap(opt1, opt2);
        EXPECT_TRUE(opt1);
        EXPECT_EQ(*opt1, "Origin");
        EXPECT_FALSE(opt2);
    }
    {
        Optional<std::string> opt1("Origin");
        Optional<std::string> opt2("New");

        swap(opt1, opt2);
        EXPECT_TRUE(opt1);
        EXPECT_EQ(*opt1, "New");
        EXPECT_TRUE(opt2);
        EXPECT_EQ(*opt2, "Origin");
    }
    {
        Optional<std::string> opt;
        EXPECT_EQ(opt.value_or("Default"), "Default");
        opt.emplace();
        EXPECT_EQ(opt.value_or("Default"), "");

        opt.emplace("Emplace");
        EXPECT_EQ(opt.value(), "Emplace");
        opt.emplace("Emplace", 3);
        EXPECT_EQ(opt.value(), "Emp");

        opt.reset();
        EXPECT_FALSE(opt.has_value());
    }
    {
        Optional<int> opt;
        opt.emplace(1);
        EXPECT_EQ(opt.value(), 1);
        opt.emplace();
        EXPECT_EQ(opt.value(), 0);
    }
    std::optional<int> a;
}

TEST(OptionalTest, CompareTest) {
    // ==
    EXPECT_TRUE(Optional<int>() == Optional<int>());
    EXPECT_FALSE(Optional<int>() == Optional<int>(0));
    EXPECT_FALSE(Optional<int>(0) == Optional<int>());
    EXPECT_FALSE(Optional<int>(0) == Optional<int>(1));
    EXPECT_FALSE(Optional<int>(1) == Optional<int>(0));
    EXPECT_TRUE(Optional<int>(1) == Optional<int>(1));
    // !=
    EXPECT_FALSE(Optional<int>() != Optional<int>());
    EXPECT_TRUE(Optional<int>() != Optional<int>(0));
    EXPECT_TRUE(Optional<int>(0) != Optional<int>());
    EXPECT_TRUE(Optional<int>(0) != Optional<int>(1));
    EXPECT_TRUE(Optional<int>(1) != Optional<int>(0));
    EXPECT_FALSE(Optional<int>(1) != Optional<int>(1));
    // <
    EXPECT_FALSE(Optional<int>() < Optional<int>());
    EXPECT_TRUE(Optional<int>() < Optional<int>(0));
    EXPECT_FALSE(Optional<int>(0) < Optional<int>());
    EXPECT_TRUE(Optional<int>(0) < Optional<int>(1));
    EXPECT_FALSE(Optional<int>(1) < Optional<int>(0));
    EXPECT_FALSE(Optional<int>(1) < Optional<int>(1));
    // >
    EXPECT_FALSE(Optional<int>() > Optional<int>());
    EXPECT_FALSE(Optional<int>() > Optional<int>(0));
    EXPECT_TRUE(Optional<int>(0) > Optional<int>());
    EXPECT_FALSE(Optional<int>(0) > Optional<int>(1));
    EXPECT_TRUE(Optional<int>(1) > Optional<int>(0));
    EXPECT_FALSE(Optional<int>(1) > Optional<int>(1));
    // <=
    EXPECT_TRUE(Optional<int>() <= Optional<int>());
    EXPECT_TRUE(Optional<int>() <= Optional<int>(0));
    EXPECT_FALSE(Optional<int>(0) <= Optional<int>());
    EXPECT_TRUE(Optional<int>(0) <= Optional<int>(1));
    EXPECT_FALSE(Optional<int>(1) <= Optional<int>(0));
    EXPECT_TRUE(Optional<int>(1) <= Optional<int>(1));
    // >=
    EXPECT_TRUE(Optional<int>() >= Optional<int>());
    EXPECT_FALSE(Optional<int>() >= Optional<int>(0));
    EXPECT_TRUE(Optional<int>(0) >= Optional<int>());
    EXPECT_FALSE(Optional<int>(0) >= Optional<int>(1));
    EXPECT_TRUE(Optional<int>(1) >= Optional<int>(0));
    EXPECT_TRUE(Optional<int>(1) >= Optional<int>(1));

    // ==
    EXPECT_TRUE(Optional<int>() == nullopt);
    EXPECT_TRUE(nullopt == Optional<int>());
    EXPECT_FALSE(Optional<int>(1) == nullopt);
    EXPECT_FALSE(nullopt == Optional<int>(1));
    // !=
    EXPECT_FALSE(Optional<int>() != nullopt);
    EXPECT_FALSE(nullopt != Optional<int>());
    EXPECT_TRUE(Optional<int>(1) != nullopt);
    EXPECT_TRUE(nullopt != Optional<int>(1));
    // <
    EXPECT_FALSE(Optional<int>() < nullopt);
    EXPECT_FALSE(nullopt < Optional<int>());
    EXPECT_FALSE(Optional<int>(1) < nullopt);
    EXPECT_TRUE(nullopt < Optional<int>(1));
    // >
    EXPECT_FALSE(Optional<int>() > nullopt);
    EXPECT_FALSE(nullopt > Optional<int>());
    EXPECT_TRUE(Optional<int>(1) > nullopt);
    EXPECT_FALSE(nullopt > Optional<int>(1));
    // <=
    EXPECT_TRUE(Optional<int>() <= nullopt);
    EXPECT_TRUE(nullopt <= Optional<int>());
    EXPECT_FALSE(Optional<int>(1) <= nullopt);
    EXPECT_TRUE(nullopt <= Optional<int>(1));
    // >=
    EXPECT_TRUE(Optional<int>() >= nullopt);
    EXPECT_TRUE(nullopt >= Optional<int>());
    EXPECT_TRUE(Optional<int>(1) >= nullopt);
    EXPECT_FALSE(nullopt >= Optional<int>(1));
}

TEST(OptionalTest, AndThenTest) {
    // and_then
    {
        Optional<int> opt;
        auto opt2 = opt.and_then([](int i) -> Optional<int> { return i + 1; });
        EXPECT_FALSE(opt2);
    }
    {
        Optional opt{42};
        auto opt2 =
            opt.and_then([](int i) -> Optional<int> { return i + 1; }).and_then([](int i) -> Optional<int> {
                return i + 1;
            });
        EXPECT_TRUE(opt2);
        EXPECT_EQ(*opt2, 44);
    }
    // transform
    {
        Optional<int> opt;
        auto opt2 = opt.transform([](int i) { return i + 1; });
        EXPECT_FALSE(opt2);
    }
    {
        Optional opt{42};
        auto opt2 = opt.transform([](int i) { return i + 1; }).transform([](int i) { return i + 1; });
        EXPECT_TRUE(opt2);
        EXPECT_EQ(*opt2, 44);
    }
    // or_else
    {
        Optional<int> opt;
        auto opt2 = opt.or_else([]() { return 10086; });
        EXPECT_EQ(opt2.value(), 10086);
    }
    {
        Optional opt{42};
        auto opt2 = opt.or_else([]() { return 10086; });
        EXPECT_TRUE(opt2);
        EXPECT_EQ(*opt2, 42);
    }
}

}  // namespace pycstl
