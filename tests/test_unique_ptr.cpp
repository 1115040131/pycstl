#include <stdio.h>

#include <format>
#include <iostream>

#include <gtest/gtest.h>

#include "unique_ptr.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

using namespace pycstl;

template <>
struct DefaultDeleter<FILE> {
    void operator()(FILE* ptr) const {
        fclose(ptr);
        std::cout << "DefaultDeleter<FILE>\n";
    }
};

struct FileDeleter {
    void operator()(FILE* ptr) const {
        fclose(ptr);
        std::cout << "FileDeleter\n";
    }
};

TEST(test_unique_ptr, test_deconstruction) {
    testing::internal::CaptureStdout();
    {
        auto file1 = UniquePtr<FILE>(fopen("myfile.txt", "w"));
        auto file2 = UniquePtr<FILE, FileDeleter>(fopen("myfile2.txt", "w"));
    }
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "FileDeleter\nDefaultDeleter<FILE>\n");

    auto p = UniquePtr<int>(new int(42));
}

struct MyClass {
    int a, b, c;
};

TEST(test_unique_ptr, test_make_unique) {
    auto int_val = makeUnique<int>();
    EXPECT_EQ(*int_val, 0);

    auto int_val2 = makeUniqueForOverwrite<int>();

    auto obj1 = makeUnique<MyClass>();
    EXPECT_EQ(obj1->a, 0);
    EXPECT_EQ(obj1->b, 0);
    EXPECT_EQ(obj1->c, 0);

    auto obj2 = makeUnique<MyClass>(12, 35, 27);
    EXPECT_EQ(obj2->a, 12);
    EXPECT_EQ(obj2->b, 35);
    EXPECT_EQ(obj2->c, 27);
}

struct Animal {
    virtual ~Animal() = default;

    virtual void speak() = 0;
};

struct Dog : Animal {
    int age;

    Dog(int _age) : age(_age) {}

    virtual void speak() {
        std::cout << std::format("Bark! I'm {} year old!\n", age);
    }
};

struct Cat : Animal {
    int& age;

    Cat(int& _age) : age(_age) {}

    virtual void speak() {
        std::cout << std::format("Meow! I'm {} year old!\n", age);
    }
};

TEST(test_unique_ptr, test_unique_ptr_vector) {
    std::vector<UniquePtr<Animal>> zoo;
    int age = 3;
    zoo.push_back(makeUnique<Cat>(age));
    zoo.push_back(makeUnique<Dog>(age));

    testing::internal::CaptureStdout();
    for (const auto& animal : zoo) {
        animal->speak();
    }
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "Meow! I'm 3 year old!\nBark! I'm 3 year old!\n");

    // Cat 通过引用绑定 age， Dog 通过值绑定
    age++;
    testing::internal::CaptureStdout();
    for (const auto& animal : zoo) {
        animal->speak();
    }
    output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "Meow! I'm 4 year old!\nBark! I'm 3 year old!\n");
}

void public_fclose(FILE* fp /* take ownership */) {
    EXPECT_EQ(fclose(fp), 0);
}

TEST(test_unique_ptr, test_ownership) {
    testing::internal::CaptureStdout();
    {
        UniquePtr<FILE> fp(fopen("myfile.txt", "r"));
        public_fclose(fp.release());
    }

    // 在析构前提前释放
    UniquePtr<FILE> fp(fopen("myfile.txt", "r"));
    fp = nullptr;
    UniquePtr<FILE> fp2(fopen("myfile.txt", "r"));
    fp2.reset();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "DefaultDeleter<FILE>\nDefaultDeleter<FILE>\n");
}
