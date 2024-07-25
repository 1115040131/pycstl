#include <fmt/core.h>
#include <gtest/gtest.h>

#include "common/json.h"

namespace pyc {

struct Address {
    std::string country{};
    std::string province{};
    std::string city{};

    auto operator<=>(const Address&) const = default;

    void show() const {
        fmt::println("Country: {}", country);
        fmt::println("Province: {}", province);
        fmt::println("City: {}", city);
    }

    std::string to_str() const { return fmt::format("{} {} {}", country, province, city); }

    REFLECT(country, province, city, show, to_str)
};

struct Student {
    std::string name{};
    int age{};
    Address addr{};

    auto operator<=>(const Student&) const = default;

    REFLECT(name, age, addr)
};

struct BigStruct {
    Student student{};
    std::vector<Address> addresses{};
    std::map<std::string, Address> address_map{};

    std::strong_ordering operator<=>(const BigStruct&) const = default;

    REFLECT(student, addresses, address_map)
};

TEST(JsonTest, JsonTest) {
    BigStruct big_struct{
        .student =
            {
                .name = "Alice",
                .age = 20,
                .addr = {.country = "China", .province = "Jiangsu", .city = "Suzhou"},
            },
        .addresses =
            {
                {.country = "China", .province = "Jiangsu", .city = "Suzhou"},
                {.country = "China", .province = "Jiangsu", .city = "Nanjing"},
            },
        .address_map =
            {
                {"home", {.country = "China", .province = "Jiangsu", .city = "Suzhou"}},
                {"school", {.country = "China", .province = "Jiangsu", .city = "Nanjing"}},
            },
    };
    {
        nlohmann::json json = big_struct;
        auto big_struct2 = json.get<BigStruct>();
        EXPECT_EQ(big_struct2, big_struct);
        EXPECT_EQ(json.dump(), static_cast<nlohmann::json>(big_struct2).dump());
    }
    fmt::println("{}", static_cast<nlohmann::json>(big_struct).dump(4));
    {
        auto big_struct_json = serialize(big_struct);
        auto big_struct2 = deserialize<BigStruct>(big_struct_json);
        EXPECT_EQ(big_struct2, big_struct);
        EXPECT_EQ(big_struct_json, serialize(big_struct2));
    }
}

}  // namespace pyc
