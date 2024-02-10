#include <fmt/core.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

TEST(JsonTest, ParseTest) {
    nlohmann::json root;
    root["id"] = 1001;
    root["data"] = "Hello world";
    std::string request = root.dump();
    fmt::println("request is {}", request);

    auto reader = nlohmann::json::parse(request);
    EXPECT_EQ(reader.at("id"), 1001);
    EXPECT_EQ(reader.at("data"), "Hello world");
}