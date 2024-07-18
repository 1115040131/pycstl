#include <map>
#include <string>
#include <string_view>

#include <fmt/core.h>
#include <gtest/gtest.h>

namespace pyc {

struct BigThing {
    BigThing(std::string_view v) : v_(v) { fmt::println("BigThing constructed {}", v_); }

    std::string v_;
};

using MyMap = std::map<std::string, BigThing>;

template <typename Key, typename Value>
void print_map(const std::map<Key, Value>& m) {
    for (const auto& [k, v] : m) {
        fmt::print("[{}: {}] ", k, v);
    }
    fmt::print("\n");
}

template <typename Key>
void print_map(const std::map<Key, BigThing>& m) {
    for (const auto& [k, v] : m) {
        fmt::print("[{}: {}] ", k, v.v_);
    }
    fmt::print("\n");
}

TEST(MapTest, MapInsertTest) {
    MyMap m;
    m.emplace("Miles", "Trumpet");
    m.emplace("Hendrix", "Guitar");
    m.emplace("Krupa", "Drums");
    m.emplace("Zappa", "Guitar");
    m.emplace("Liszt", "Piano");
    print_map(m);
    m.emplace("Hendrix", "Singer");
    EXPECT_FALSE(
        m.try_emplace("Zappa", "Composer").second);  // 有重复键 try_emplace 失败, 不调用 BigThing 构造函数
}

template <typename Key, typename Value>
bool node_swap(std::map<Key, Value>& m, Key k1, Key k2) {
    // extract 将节点保留在原处的同时将其解除关联, 并返回一个 node_type
    // node_type 有一个成员函数 key(), 返回一个对节点键的可写引用
    auto node1 = m.extract(k1);
    auto node2 = m.extract(k2);
    // 若没有找到键, 返回一个空节点, 用 empty 测试节点是否为空
    if (node1.empty() || node2.empty()) {
        return false;
    }
    std::swap(node1.key(), node2.key());
    m.insert(std::move(node1));
    m.insert(std::move(node2));
    return true;
}

TEST(MapTest, MapModifyTest) {
    std::map<int, std::string> racers{
        {1, "Mario"}, {2, "Luigi"}, {3, "Bowser"}, {4, "Peach"}, {5, "Donkey Kong Jr"},
    };
    print_map(racers);
    EXPECT_TRUE(node_swap(racers, 3, 5));
    print_map(racers);

    // 插入已有的键会失败
    auto node{racers.extract(racers.begin())};
    node.key() = 5;
    EXPECT_FALSE(racers.insert(std::move(node)).inserted);
    print_map(racers);
};

}  // namespace pyc
