#include <fmt/base.h>
#include <gtest/gtest.h>

#include "monkey/compiler/symbol_table.h"

namespace pyc {
namespace monkey {

struct SymbolTableTestCase {
    std::string name;
    Symbol expected;
};

TEST(SymbolTableTest, DefineTest) {
    std::vector<SymbolTableTestCase> tests = {
        {"a", {"a", SymbolScopeType::kGlobal, 0}},
        {"b", {"b", SymbolScopeType::kGlobal, 1}},
        {"c", {"c", SymbolScopeType::kGlobal, 2}},
    };

    auto symbol_table = SymbolTable::New();
    for (const auto& test : tests) {
        auto symbol = symbol_table->Define(test.name);
        ASSERT_TRUE(symbol) << "Failed to define symbol: " << test.name;
        EXPECT_EQ(*symbol, test.expected) << "Symbol mismatch for: " << test.name;
    }
}

TEST(SymbolTableTest, ResolveTest) {
    std::vector<SymbolTableTestCase> tests = {
        {"a", {"a", SymbolScopeType::kGlobal, 0}},
        {"b", {"b", SymbolScopeType::kGlobal, 1}},
        {"c", {"c", SymbolScopeType::kGlobal, 2}},
    };

    auto symbol_table = SymbolTable::New();
    for (const auto& test : tests) {
        auto symbol = symbol_table->Define(test.name);
    }

    for (const auto& test : tests) {
        auto symbol = symbol_table->Resolve(test.name);
        ASSERT_TRUE(symbol) << "Failed to resolve symbol: " << test.name;
        EXPECT_EQ(*symbol, test.expected) << "Symbol mismatch for: " << test.name;
    }
}

}  // namespace monkey
}  // namespace pyc