#include <gtest/gtest.h>

#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

TEST(ObjectTest, StringHashTest) {
    auto hello1 = String("Hello World");
    auto hello2 = String("Hello World");

    auto diff1 = String("My name is Leslie");
    auto diff2 = String("My name is Leslie");

    EXPECT_EQ(hello1.getHashKey(), hello2.getHashKey());
    EXPECT_EQ(diff1.getHashKey(), diff2.getHashKey());

    EXPECT_NE(hello1.getHashKey(), diff1.getHashKey());
}

}  // namespace monkey
}  // namespace pyc
