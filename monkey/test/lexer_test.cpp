#include <gtest/gtest.h>

#include "monkey/token/token.h"

namespace pyc {
namespace monkey {

TEST(MonkeyTest, ToStringTest) {
    EXPECT_EQ(std::string(toString(Token::Type::kIllegal)), "ILLEGAL");
    EXPECT_EQ(std::string(toString(Token::Type::kEof)), "EOF");
    EXPECT_EQ(std::string(toString(Token::Type::kIdent)), "IDENT");
    EXPECT_EQ(std::string(toString(Token::Type::kInt)), "INT");
}

}  // namespace monkey
}  // namespace pyc
