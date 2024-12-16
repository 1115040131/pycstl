#include <atomic>

#include <fmt/base.h>
#include <gtest/gtest.h>

#include "pycstl/shared_ptr.h"

namespace pycstl {

static std::atomic<int> g_student_count = 0;

struct Student : public EnableSharedFromThis<Student> {
    std::string_view name;
    int age;

    Student(std::string_view name, int age) : name(name), age(age) {
        fmt::println("Student {} 构造", name);
        g_student_count++;
    }

    virtual ~Student() {
        fmt::println("Student {} 析构", name);
        g_student_count--;
    }

    void func() { fmt::println("{}", reinterpret_cast<void*>(shared_from_this().get())); }
};

struct StudentDerived : public Student {
    using Student::Student;

    ~StudentDerived() override { fmt::println("StudentDerived {} 析构", name); }
};

TEST(SharedPtrTest, ConstructionTest) {
    {
        SharedPtr student = MakeShared<Student>("彭于晏", 23);
        SharedPtr<Student> student2(new StudentDerived("彭于晏", 23));
        SharedPtr<Student> student3(new Student("彭于晏", 23), [](Student* ptr) { delete ptr; });
        SharedPtr student4 = student;
        SharedPtr student5 = student2;

        EXPECT_EQ(student.use_count(), 2);
        EXPECT_EQ(student->name, "彭于晏");
        EXPECT_EQ(student->age, 23);
        EXPECT_EQ(student2.use_count(), 2);
        EXPECT_EQ(student2->name, "彭于晏");
        EXPECT_EQ(student2->age, 23);
        EXPECT_EQ(student3.use_count(), 1);
        EXPECT_EQ(student3->name, "彭于晏");
        EXPECT_EQ(student3->age, 23);
        EXPECT_EQ(student4.use_count(), 2);
        EXPECT_EQ(student4->name, "彭于晏");
        EXPECT_EQ(student4->age, 23);
        EXPECT_EQ(student5.use_count(), 2);
        EXPECT_EQ(student5->name, "彭于晏");
        EXPECT_EQ(student5->age, 23);

        EXPECT_EQ(g_student_count, 3);
    }
    EXPECT_EQ(g_student_count, 0);
    {
        UniquePtr<Student> student1(new Student("彭于晏", 23));
        SharedPtr<Student> student2(std::move(student1));

        EXPECT_EQ(student2.use_count(), 1);
        EXPECT_EQ(student2->name, "彭于晏");
        EXPECT_EQ(student2->age, 23);

        EXPECT_EQ(g_student_count, 1);
    }
    EXPECT_EQ(g_student_count, 0);
    {
        UniquePtr<StudentDerived> student1(new StudentDerived("彭于晏", 23));
        SharedPtr<Student> student2(std::move(student1));

        EXPECT_EQ(student2.use_count(), 1);
        EXPECT_EQ(student2->name, "彭于晏");
        EXPECT_EQ(student2->age, 23);

        EXPECT_EQ(g_student_count, 1);
    }
    EXPECT_EQ(g_student_count, 0);
}

TEST(SharedPtrTest, VoidTest) {
    {
        SharedPtr<void> ptr(new Student("彭于晏", 23));
        EXPECT_EQ(g_student_count, 1);
    }
    EXPECT_EQ(g_student_count, 0);
}

TEST(SharedPtrTest, StaticPointerCastTest) {
    {
        SharedPtr<void> ptr(new StudentDerived("彭于晏", 23));
        SharedPtr<Student> student = StaticPointerCast<Student>(ptr);
        EXPECT_EQ(g_student_count, 1);
    }
    EXPECT_EQ(g_student_count, 0);
}

TEST(SharedPtrTest, ConstPointerCastTest) {
    {
        SharedPtr<Student> ptr(new StudentDerived("彭于晏", 23));
        SharedPtr<const Student> const_ptr = ptr;
        ptr = ConstPointerCast<Student>(const_ptr);
        EXPECT_EQ(g_student_count, 1);
    }
    EXPECT_EQ(g_student_count, 0);
}

TEST(SharedPtrTest, ImplicitConvertTest) {
    {
        SharedPtr<Student> ptr(new StudentDerived("彭于晏", 23));
        auto dp = StaticPointerCast<StudentDerived>(ptr);
        SharedPtr<const Student> bp = dp;
        EXPECT_EQ(g_student_count, 1);
    }
    EXPECT_EQ(g_student_count, 0);
}

TEST(SharedPtrTest, EnableSharedFromThisTest) {
    {
        SharedPtr student1 = MakeShared<Student>("彭于晏", 23);
        SharedPtr<Student> student2(new StudentDerived("彭于晏", 23));
        EXPECT_EQ(g_student_count, 2);

        student1->func();

        EXPECT_THROW(student2->func(), std::bad_weak_ptr);
    }
    EXPECT_EQ(g_student_count, 0);
}

}  // namespace pycstl
