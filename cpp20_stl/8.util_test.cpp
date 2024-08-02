#include <any>
#include <chrono>
#include <list>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <fmt/base.h>
#include <fmt/chrono.h>
#include <gtest/gtest.h>

namespace pyc {

using namespace std::string_literals;

#pragma region std::any

std::string dump_any(const std::any& a) {
    if (!a.has_value()) {
        return "None.";
    } else if (a.type() == typeid(int)) {
        return fmt::format("int: {}", std::any_cast<int>(a));
    } else if (a.type() == typeid(std::string)) {
        return fmt::format("string: {}", std::any_cast<std::string>(a));
    } else if (a.type() == typeid(std::list<int>)) {
        std::string result = "list<int>: ";
        // 可以使用 any_cast<T&>() 来返回引用
        for (const auto& i : std::any_cast<const std::list<int>&>(a)) {
            result += fmt::format("{} ", i);
        }
        return result;
    }
    return fmt::format("Something else: {}", a.type().name());
}

void print_any(const std::any& a) { fmt::println("{}", dump_any(a)); }

TEST(UtilTest, AnyTest) {
    std::any x{};
    EXPECT_FALSE(x.has_value());

    x = 42;
    EXPECT_TRUE(x.has_value());
    EXPECT_EQ(x.type().name(), typeid(int).name());
    EXPECT_EQ(std::any_cast<int>(x), 42);

    x = "abc"s;
    EXPECT_TRUE(x.has_value());
    EXPECT_EQ(x.type().name(), typeid(std::string).name());
    EXPECT_EQ(std::any_cast<std::string>(x), "abc");

    EXPECT_EQ(dump_any({}), "None.");
    EXPECT_EQ(dump_any(47), "int: 47");
    EXPECT_EQ(dump_any("abc"s), "string: abc");
    EXPECT_EQ(dump_any(std::list{1, 2, 3}), "list<int>: 1 2 3 ");
    EXPECT_EQ(dump_any(std::vector{1, 2, 3}), "Something else: St6vectorIiSaIiEE");

    // 若用错误的类型强制转换 any 对象，则会抛出一个 bad_any_cast 异常
    EXPECT_THROW(std::any_cast<int>(x), std::bad_any_cast);
}

#pragma endregion

#pragma region std::variant

TEST(UtilTest, VariantTest) {
    class Animal {
    public:
        Animal(std::string_view name, std::string_view sound) : name_(name), sound_(sound) {}

        std::string speak() const { return fmt::format("{} says {}", name_, sound_); }

        void sound(std::string_view sound) { sound_ = sound; }

    private:
        std::string_view name_;
        std::string_view sound_;
    };

    class Cat : public Animal {
    public:
        Cat(std::string_view name) : Animal(name, "meow") {}
    };

    class Dog : public Animal {
    public:
        Dog(std::string_view name) : Animal(name, "arf!") {}
    };

    class Wookie : public Animal {
    public:
        Wookie(std::string_view name) : Animal(name, "grrraarrgghh!") {}
    };

    using v_animal = std::variant<Cat, Dog, Wookie>;

    std::list<v_animal> pets{
        Cat{"Hobbes"},
        Dog{"Fido"},
        Cat{"Max"},
        Wookie{"Chewie"},
    };

    std::vector<std::string> expected{
        "Hobbes says meow",
        "Fido says arf!",
        "Max says meow",
        "Chewie says grrraarrgghh!",
    };

    {
        // visit() 调用带有当前包含在变体中的对象的函子
        // 定义一个接受宠物的函子
        struct animal_speaks {
            std::string operator()(const Dog& d) const { return d.speak(); }
            std::string operator()(const Cat& c) const { return c.speak(); }
            std::string operator()(const Wookie& w) const { return w.speak(); }
        };

        std::vector<std::string> result;
        for (const auto& pet : pets) {
            result.push_back(std::visit(animal_speaks{}, pet));
        }
        EXPECT_EQ(result, expected);
    }
    {
        // variant 类还提供了一个 index() 方法
        std::vector<std::string> result;
        for (const auto& pet : pets) {
            auto index{pet.index()};
            if (index == 0) {
                result.push_back(std::get<Cat>(pet).speak());
            } else if (index == 1) {
                result.push_back(std::get<Dog>(pet).speak());
            } else if (index == 2) {
                result.push_back(std::get<Wookie>(pet).speak());
            }
        }
        EXPECT_EQ(result, expected);
    }
    {
        // get_if<T>() 函数的作用是: 根据类型测试给定元素
        std::vector<std::string> result;
        for (const auto& pet : pets) {
            if (auto cat = std::get_if<Cat>(&pet)) {
                result.push_back(cat->speak());
            } else if (auto dog = std::get_if<Dog>(&pet)) {
                result.push_back(dog->speak());
            } else if (auto wookie = std::get_if<Wookie>(&pet)) {
                result.push_back(wookie->speak());
            }
        }
        EXPECT_EQ(result, expected);
    }
    // hold_alternative <T>() 函数返回 true 或 false
    int animal_count[3]{};
    for (const auto& pet : pets) {
        if (std::holds_alternative<Cat>(pet)) {
            ++animal_count[0];
        } else if (std::holds_alternative<Dog>(pet)) {
            ++animal_count[1];
        } else if (std::holds_alternative<Wookie>(pet)) {
            ++animal_count[2];
        }
    }
    EXPECT_EQ(animal_count[0], 2);
    EXPECT_EQ(animal_count[1], 1);
    EXPECT_EQ(animal_count[2], 1);
}

#pragma endregion

#pragma region std::chrono

uint64_t count_primes() {
    constexpr auto is_prime = [](const uint64_t n) {
        for (uint64_t i{2}; i < n / 2; ++i) {
            if (n % i == 0) {
                return false;
            }
        }
        return true;
    };
    uint64_t count{0};
    constexpr uint64_t kStart{2};
    constexpr uint64_t kMaxPrime{0x1ffff};
    for (uint64_t i{kStart}; i <= kMaxPrime; ++i) {
        if (is_prime(i)) {
            ++count;
        }
    }
    return count;
}

// steady_clock 设计用于提供可靠一致的单调，适用于计时事件。它使用一个相对的时间参考，
// system_clock 从固定的时间点 (1970 年 1 月 1 日 00:00 UTC) 开始测量
template <typename F>
std::pair<std::chrono::duration<double>, std::invoke_result_t<F>> timer(F&& f) {
    auto start = std::chrono::system_clock::now();
    auto result = f();
    auto end = std::chrono::system_clock::now();
    return {end - start, result};
}

TEST(UtilTest, ChronoTest) {
    auto now = std::chrono::system_clock::now();

    fmt::println("system_clock::now is {:%F %T}", now);

    auto [duration, count] = timer(count_primes);
    EXPECT_EQ(count, 12252);
    fmt::println("find {} primes in range took {:.3f} seconds", count, duration.count());

    using milliseconds = std::chrono::duration<double, std::milli>;
    using microsecond = std::chrono::duration<double, std::micro>;
    using fps24 = std::chrono::duration<unsigned long, std::ratio<1, 24>>;
    fmt::println("time elapsed: {:.3f} ms", milliseconds(duration).count());
    fmt::println("time elapsed: {:.3e} μs", microsecond(duration).count());
    fmt::println("time elapsed: {} frames at 24 fps", std::chrono::floor<fps24>(duration).count());
}

#pragma endregion

#pragma region 对可变元组使用折叠表达式

// 折叠表达式有四种类型
//  一元右折叠: (args op ...)
//  一元左折叠: (... op args)
//  二元右折叠: (args op ... op init)
//  二元左折叠: (init op ... op args)

template <typename... Ts>
constexpr std::string format_ts(const std::tuple<Ts...>& tup) {
    std::string result;
    auto lpt = [&tup, &result]<size_t... I>(std::index_sequence<I...>) constexpr {
        (..., (result += fmt::format(I ? ", {}" : "{}", std::get<I>(tup))));
        result += "\n";
    };
    lpt(std::make_index_sequence<sizeof...(Ts)>{});
    return result;
}

template <typename... Ts>
constexpr int sum_ts(const std::tuple<Ts...>& tup) {
    int accum{};
    auto lpt = [&tup, &accum]<size_t... I>(std::index_sequence<I...>) constexpr {
        (..., (accum += std::get<I>(tup)));
    };
    lpt(std::make_index_sequence<sizeof...(Ts)>{});
    return accum;
}

TEST(UtilTest, FoldExpressionTest) {
    EXPECT_EQ(format_ts(std::tuple{"ID", "Name", "Scale"}), "ID, Name, Scale\n");
    EXPECT_EQ(format_ts(std::tuple{123456, "John Doe", 3.7}), "123456, John Doe, 3.7\n");
    EXPECT_EQ(format_ts(std::tuple{1, 7, "forty-two", 47, 73L, -111.11}), "1, 7, forty-two, 47, 73, -111.11\n");

    EXPECT_EQ(sum_ts(std::tuple{1, 2, 3, 4, 5}), 15);
    EXPECT_EQ(sum_ts(std::tuple{9, 10, 11, 12, 13, 14, 15}), 84);
    EXPECT_EQ(sum_ts(std::tuple{47, 73, 42}), 162);
}

#pragma endregion

#pragma region 共享管理对象的成员

class Animal {
public:
    Animal(std::string_view name, std::string_view sound) : name_(name), sound_(sound) {
        fmt::print("ctor: {}\n", name_);
    }

    ~Animal() { fmt::print("dtor: {}\n", name_); }

    std::string_view name_;
    std::string_view sound_;
};

auto make_animal(std::string_view name, std::string_view sound) {
    auto animal = std::make_shared<Animal>(name, sound);
    auto name_ptr = std::shared_ptr<std::string_view>(animal, &animal->name_);
    auto sound_ptr = std::shared_ptr<std::string_view>(animal, &animal->sound_);
    return std::make_tuple(name_ptr, sound_ptr);
}

TEST(UtilTest, SharedMemberTest) {
    auto [name, sound] = make_animal("Velociraptor", "Grrrr!");
    fmt::println("The {} says {}", *name, *sound);
    EXPECT_EQ(name.use_count(), 2);
    EXPECT_EQ(sound.use_count(), 2);
}

#pragma endregion

}  // namespace pyc
