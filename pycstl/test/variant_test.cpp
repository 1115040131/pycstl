#include <string>

#include <fmt/base.h>
#include <fmt/format.h>
#include <gtest/gtest.h>

#include "pycstl/variant.h"

namespace pycstl {

class VariantTest : public ::testing::Test {
public:
    struct Point {
        int x;
        int y;

        auto operator<=>(const Point&) const = default;
    };

    using VariantType = Variant<std::string, int, double, Point>;

    const VariantType v0_{std::string("abcd")};
    const VariantType v1_{6};
    const VariantType v2_{3.1415};
    const VariantType v3_{Point{4, 8}};
};

}  // namespace pycstl

namespace fmt {

template <>
struct formatter<pycstl::VariantTest::Point> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const pycstl::VariantTest::Point& point, FormatContext& ctx) const {
        return format_to(ctx.out(), "Point{{{}, {}}}", point.x, point.y);
    }
};

}  // namespace fmt

namespace pycstl {

TEST_F(VariantTest, CopyConstructTest) {
    {
        VariantType v{v0_};
        EXPECT_EQ(v.index(), 0);
        EXPECT_EQ(v.get<std::string>(), "abcd");
    }
    {
        VariantType v{v1_};
        EXPECT_EQ(v.index(), 1);
        EXPECT_EQ(v.get<int>(), 6);
    }
    {
        VariantType v{v2_};
        EXPECT_EQ(v.index(), 2);
        EXPECT_EQ(v.get<double>(), 3.1415);
    }
    {
        VariantType v{v3_};
        EXPECT_EQ(v.index(), 3);
        EXPECT_EQ(v.get<Point>(), (Point{4, 8}));
    }
}

TEST_F(VariantTest, MoveConstructTest) {
    {
        auto v0 = v0_;
        VariantType v{std::move(v0)};
        EXPECT_EQ(v.index(), 0);
        EXPECT_EQ(v.get<std::string>(), "abcd");
    }
    {
        auto v1 = v1_;
        VariantType v{std::move(v1)};
        EXPECT_EQ(v.index(), 1);
        EXPECT_EQ(v.get<int>(), 6);
    }
    {
        auto v2 = v2_;
        VariantType v{std::move(v2)};
        EXPECT_EQ(v.index(), 2);
        EXPECT_EQ(v.get<double>(), 3.1415);
    }
    {
        auto v3 = v3_;
        VariantType v{std::move(v3)};
        EXPECT_EQ(v.index(), 3);
        EXPECT_EQ(v.get<Point>(), (Point{4, 8}));
    }
}

TEST_F(VariantTest, CopyAssignmentTest) {
    VariantType v{std::string("abcd")};
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<std::string>(), "abcd");

    v = v1_;
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<int>(), 6);

    v = v2_;
    EXPECT_EQ(v.index(), 2);
    EXPECT_EQ(v.get<double>(), 3.1415);

    v = v3_;
    EXPECT_EQ(v.index(), 3);
    EXPECT_EQ(v.get<Point>(), (Point{4, 8}));

    v = v0_;
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<std::string>(), "abcd");
}

TEST_F(VariantTest, MoveAssignmentTest) {
    VariantType v{std::string("abcd")};
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<std::string>(), "abcd");

    auto v1 = v1_;
    v = std::move(v1);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<int>(), 6);

    auto v2 = v2_;
    v = std::move(v2);
    EXPECT_EQ(v.index(), 2);
    EXPECT_EQ(v.get<double>(), 3.1415);

    auto v3 = v3_;
    v = std::move(v3);
    EXPECT_EQ(v.index(), 3);
    EXPECT_EQ(v.get<Point>(), (Point{4, 8}));

    auto v0 = v0_;
    v = std::move(v0);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<std::string>(), "abcd");
}

TEST_F(VariantTest, ConstructTest) {
    {
        auto v = VariantType(InPlaceIndex<0>(), "abcd");
        EXPECT_EQ(v.index(), 0);
        EXPECT_TRUE(v.holds_alternative<std::string>());
        EXPECT_EQ(v.get<std::string>(), "abcd");
    }
    {
        auto v = VariantType(InPlaceIndex<1>(), 6);
        EXPECT_EQ(v.index(), 1);
        EXPECT_TRUE(v.holds_alternative<int>());
        EXPECT_EQ(v.get<int>(), 6);
    }
    {
        auto v = VariantType(InPlaceIndex<2>(), 3.1415);
        EXPECT_EQ(v.index(), 2);
        EXPECT_TRUE(v.holds_alternative<double>());
        EXPECT_EQ(v.get<double>(), 3.1415);
    }
    {
        auto v = VariantType(InPlaceIndex<3>(), 4, 8);
        EXPECT_EQ(v.index(), 3);
        EXPECT_TRUE(v.holds_alternative<Point>());
        EXPECT_EQ(v.get<Point>(), (Point{4, 8}));
    }
}

TEST_F(VariantTest, IndexTest) {
    EXPECT_EQ(v0_.index(), 0);
    EXPECT_EQ(v1_.index(), 1);
    EXPECT_EQ(v2_.index(), 2);
}

TEST_F(VariantTest, HoldsAlernativeTest) {
    EXPECT_TRUE(v0_.holds_alternative<std::string>());
    EXPECT_FALSE(v0_.holds_alternative<int>());
    EXPECT_FALSE(v0_.holds_alternative<double>());

    EXPECT_FALSE(v1_.holds_alternative<std::string>());
    EXPECT_TRUE(v1_.holds_alternative<int>());
    EXPECT_FALSE(v1_.holds_alternative<double>());

    EXPECT_FALSE(v2_.holds_alternative<std::string>());
    EXPECT_FALSE(v2_.holds_alternative<int>());
    EXPECT_TRUE(v2_.holds_alternative<double>());
}

TEST_F(VariantTest, GetTest) {
    EXPECT_EQ(v0_.get<0>(), "abcd");
    EXPECT_THROW(v0_.get<1>(), BadVariantAccess);
    EXPECT_THROW(v0_.get<2>(), BadVariantAccess);
    EXPECT_EQ(v0_.get<std::string>(), "abcd");
    EXPECT_THROW(v0_.get<int>(), BadVariantAccess);
    EXPECT_THROW(v0_.get<double>(), BadVariantAccess);

    EXPECT_THROW(v1_.get<0>(), BadVariantAccess);
    EXPECT_EQ(v1_.get<1>(), 6);
    EXPECT_THROW(v1_.get<2>(), BadVariantAccess);
    EXPECT_THROW(v1_.get<std::string>(), BadVariantAccess);
    EXPECT_EQ(v1_.get<int>(), 6);
    EXPECT_THROW(v1_.get<double>(), BadVariantAccess);

    EXPECT_THROW(v2_.get<0>(), BadVariantAccess);
    EXPECT_THROW(v2_.get<1>(), BadVariantAccess);
    EXPECT_EQ(v2_.get<2>(), 3.1415);
    EXPECT_THROW(v2_.get<std::string>(), BadVariantAccess);
    EXPECT_THROW(v2_.get<int>(), BadVariantAccess);
    EXPECT_EQ(v2_.get<double>(), 3.1415);
}

TEST_F(VariantTest, GetIfTest) {
    EXPECT_EQ(*v0_.get_if<0>(), "abcd");
    EXPECT_EQ(v0_.get_if<1>(), nullptr);
    EXPECT_EQ(v0_.get_if<2>(), nullptr);
    EXPECT_EQ(v0_.get_if<3>(), nullptr);
    EXPECT_EQ(*v0_.get_if<std::string>(), "abcd");
    EXPECT_EQ(v0_.get_if<int>(), nullptr);
    EXPECT_EQ(v0_.get_if<double>(), nullptr);
    EXPECT_EQ(v0_.get_if<Point>(), nullptr);

    EXPECT_EQ(v1_.get_if<0>(), nullptr);
    EXPECT_EQ(*v1_.get_if<1>(), 6);
    EXPECT_EQ(v1_.get_if<2>(), nullptr);
    EXPECT_EQ(v1_.get_if<3>(), nullptr);
    EXPECT_EQ(v1_.get_if<std::string>(), nullptr);
    EXPECT_EQ(*v1_.get_if<int>(), 6);
    EXPECT_EQ(v1_.get_if<double>(), nullptr);
    EXPECT_EQ(v1_.get_if<Point>(), nullptr);

    EXPECT_EQ(v2_.get_if<0>(), nullptr);
    EXPECT_EQ(v2_.get_if<1>(), nullptr);
    EXPECT_EQ(*v2_.get_if<2>(), 3.1415);
    EXPECT_EQ(v2_.get_if<3>(), nullptr);
    EXPECT_EQ(v2_.get_if<std::string>(), nullptr);
    EXPECT_EQ(v2_.get_if<int>(), nullptr);
    EXPECT_EQ(*v2_.get_if<double>(), 3.1415);
    EXPECT_EQ(v2_.get_if<Point>(), nullptr);

    EXPECT_EQ(v3_.get_if<0>(), nullptr);
    EXPECT_EQ(v3_.get_if<1>(), nullptr);
    EXPECT_EQ(v3_.get_if<2>(), nullptr);
    EXPECT_EQ(*v3_.get_if<3>(), (Point{4, 8}));
    EXPECT_EQ(v3_.get_if<std::string>(), nullptr);
    EXPECT_EQ(v3_.get_if<int>(), nullptr);
    EXPECT_EQ(v3_.get_if<double>(), nullptr);
    EXPECT_EQ(*v3_.get_if<Point>(), (Point{4, 8}));
}

void print(const VariantTest::VariantType& v) {
    v.visit([](const auto& value) {
        [[maybe_unused]] static auto dummy = printf("instanced\n");
        fmt::println("value: {}", value);
    });
}

auto test(const VariantTest::VariantType& v) {
    return v.visit([](const auto&) { return 1; });
}

TEST_F(VariantTest, VisitTest) {
    print(v0_);
    print(v1_);
    print(v2_);
    print(v3_);

    EXPECT_EQ(test(v0_), 1);
    EXPECT_EQ(test(v1_), 1);
    EXPECT_EQ(test(v2_), 1);
    EXPECT_EQ(test(v3_), 1);
}

}  // namespace pycstl