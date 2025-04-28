#include <gtest/gtest.h>

#include "monkey/ast/ast.h"
#include "monkey/lexer/lexer.h"
#include "monkey/test/test_define.h"

namespace pyc {
namespace monkey {

TEST(AstTest, ToStringTest) {
    std::string input = "let myVar = anotherVar;";
    auto lexer = Lexer::New(input);

    auto let_statement = std::make_unique<LetStatement>(lexer->nextToken());
    let_statement->setName(std::make_unique<Identifier>(lexer->nextToken()));
    EXPECT_EQ(lexer->nextToken(), (Token{Token::Type::kAssign, "="}));
    let_statement->setValue(std::make_unique<Identifier>(lexer->nextToken()));
    EXPECT_EQ(lexer->nextToken(), (Token{Token::Type::kSemicolon, ";"}));

    auto program = std::make_unique<Program>();
    program->addStatement(std::move(let_statement));
    EXPECT_EQ(program->toString(), "let myVar = anotherVar;");
}

}  // namespace monkey
}  // namespace pyc
