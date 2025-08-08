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
    std::map<std::string, Symbol> expected = {
        {"a", {"a", SymbolScopeType::kGlobal, 0}}, {"b", {"b", SymbolScopeType::kGlobal, 1}},
        {"c", {"c", SymbolScopeType::kLocal, 0}},  {"d", {"d", SymbolScopeType::kLocal, 1}},
        {"e", {"e", SymbolScopeType::kLocal, 0}},  {"f", {"f", SymbolScopeType::kLocal, 1}},
    };

    auto global = SymbolTable::New();
    auto first_local = SymbolTable::NewEnclosed(global);
    auto second_local = SymbolTable::NewEnclosed(first_local);

    auto a = global->Define("a");
    EXPECT_EQ(*a, expected["a"]);
    auto b = global->Define("b");
    EXPECT_EQ(*b, expected["b"]);

    auto c = first_local->Define("c");
    EXPECT_EQ(*c, expected["c"]);
    auto d = first_local->Define("d");
    EXPECT_EQ(*d, expected["d"]);

    auto e = second_local->Define("e");
    EXPECT_EQ(*e, expected["e"]);
    auto f = second_local->Define("f");
    EXPECT_EQ(*f, expected["f"]);
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

TEST(SymbolTableTest, ResolveLocalTest) {
    auto global = SymbolTable::New();
    global->Define("a");
    global->Define("b");

    auto local = SymbolTable::NewEnclosed(global);
    local->Define("c");
    local->Define("d");

    std::vector<SymbolTableTestCase> tests = {
        {"a", {"a", SymbolScopeType::kGlobal, 0}},
        {"b", {"b", SymbolScopeType::kGlobal, 1}},
        {"c", {"c", SymbolScopeType::kLocal, 0}},
        {"d", {"d", SymbolScopeType::kLocal, 1}},
    };

    for (const auto& test : tests) {
        auto symbol = local->Resolve(test.name);
        ASSERT_TRUE(symbol) << "Failed to resolve symbol: " << test.name;
        EXPECT_EQ(*symbol, test.expected) << "Symbol mismatch for: " << test.name;
    }
}

TEST(SymbolTableTest, ResolveNestedLocalTest) {
    auto global = SymbolTable::New();
    global->Define("a");
    global->Define("b");

    auto first_local = SymbolTable::NewEnclosed(global);
    first_local->Define("c");
    first_local->Define("d");

    auto second_local = SymbolTable::NewEnclosed(first_local);
    second_local->Define("e");
    second_local->Define("f");

    std::vector<SymbolTableTestCase> first_local_tests = {
        {"a", {"a", SymbolScopeType::kGlobal, 0}},
        {"b", {"b", SymbolScopeType::kGlobal, 1}},
        {"c", {"c", SymbolScopeType::kLocal, 0}},
        {"d", {"d", SymbolScopeType::kLocal, 1}},
    };

    for (const auto& test : first_local_tests) {
        auto symbol = first_local->Resolve(test.name);
        ASSERT_TRUE(symbol) << "Failed to resolve symbol: " << test.name;
        EXPECT_EQ(*symbol, test.expected) << "Symbol mismatch for: " << test.name;
    }
    EXPECT_FALSE(first_local->Resolve("e"));
    EXPECT_FALSE(first_local->Resolve("f"));

    std::vector<SymbolTableTestCase> second_local_tests = {
        {"a", {"a", SymbolScopeType::kGlobal, 0}}, {"b", {"b", SymbolScopeType::kGlobal, 1}},
        {"c", {"c", SymbolScopeType::kFree, 0}},   {"d", {"d", SymbolScopeType::kFree, 1}},
        {"e", {"e", SymbolScopeType::kLocal, 0}},  {"f", {"f", SymbolScopeType::kLocal, 1}},
    };

    for (const auto& test : second_local_tests) {
        auto symbol = second_local->Resolve(test.name);
        ASSERT_TRUE(symbol) << "Failed to resolve symbol: " << test.name;
        EXPECT_EQ(*symbol, test.expected) << "Symbol mismatch for: " << test.name;
    }
}

TEST(SymbolTableTest, ResolveBuiltinsTest) {
    std::map<std::string, Symbol> expected = {
        {"a", {"a", SymbolScopeType::kBuiltin, 0}},
        {"b", {"b", SymbolScopeType::kBuiltin, 1}},
        {"c", {"c", SymbolScopeType::kBuiltin, 2}},
        {"d", {"d", SymbolScopeType::kBuiltin, 3}},
    };

    auto global = SymbolTable::New();
    auto first_local = SymbolTable::NewEnclosed(global);
    auto second_local = SymbolTable::NewEnclosed(first_local);

    auto tests = std::vector{global, first_local, second_local};

    for (const auto& [name, expect] : expected) {
        global->DefineBuiltin(expect.name, expect.index);
    }

    for (const auto& test : tests) {
        for (const auto& [name, expect] : expected) {
            auto result = test->Resolve(name);
            ASSERT_TRUE(result);
            EXPECT_EQ(*result, expect);
        }
    }
}

TEST(SymbolTableTest, ResolveUnresolveableFreeTest) {
    auto global = SymbolTable::New();
    global->Define("a");

    auto first_local = SymbolTable::NewEnclosed(global);
    first_local->Define("c");

    auto second_local = SymbolTable::NewEnclosed(first_local);
    second_local->Define("e");
    second_local->Define("f");

    std::vector<SymbolTableTestCase> first_local_tests = {
        {"a", {"a", SymbolScopeType::kGlobal, 0}},
        {"c", {"c", SymbolScopeType::kLocal, 0}},
    };

    for (const auto& test : first_local_tests) {
        auto symbol = first_local->Resolve(test.name);
        ASSERT_TRUE(symbol) << "Failed to resolve symbol: " << test.name;
        EXPECT_EQ(*symbol, test.expected) << "Symbol mismatch for: " << test.name;
    }
    EXPECT_FALSE(first_local->Resolve("b"));
    EXPECT_FALSE(first_local->Resolve("d"));
    EXPECT_FALSE(first_local->Resolve("e"));
    EXPECT_FALSE(first_local->Resolve("f"));

    std::vector<SymbolTableTestCase> second_local_tests = {
        {"a", {"a", SymbolScopeType::kGlobal, 0}},
        {"c", {"c", SymbolScopeType::kFree, 0}},
        {"e", {"e", SymbolScopeType::kLocal, 0}},
        {"f", {"f", SymbolScopeType::kLocal, 1}},
    };

    for (const auto& test : second_local_tests) {
        auto symbol = second_local->Resolve(test.name);
        ASSERT_TRUE(symbol) << "Failed to resolve symbol: " << test.name;
        EXPECT_EQ(*symbol, test.expected) << "Symbol mismatch for: " << test.name;
    }
    EXPECT_FALSE(second_local->Resolve("b"));
    EXPECT_FALSE(second_local->Resolve("d"));
}

TEST(SymbolTableTest, DefineAndResolveFunctionNameTest) {
    auto global = SymbolTable::New();
    global->DefineFunctionName("a");

    auto result = global->Resolve("a");
    auto expected = Symbol{"a", SymbolScopeType::kFunctionScope, 0};
    EXPECT_EQ(*result, expected);
}

TEST(SymbolTableTest, ShadowingFunctionNameTest) {
    auto global = SymbolTable::New();
    global->DefineFunctionName("a");
    global->Define("a");

    auto result = global->Resolve("a");
    auto expected = Symbol{"a", SymbolScopeType::kGlobal, 0};
    EXPECT_EQ(*result, expected);
}

}  // namespace monkey
}  // namespace pyc