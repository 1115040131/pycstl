#include <gtest/gtest.h>

#include "monkey/parser/parser.h"

namespace fmt {

template <typename... Args>
struct formatter<std::variant<Args...>> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const std::variant<Args...>& v, FormatContext& ctx) const {
        return std::visit([&ctx](const auto& value) { return fmt::format_to(ctx.out(), "{}", value); }, v);
    }
};

}  // namespace fmt

namespace pyc {
namespace monkey {

using ValueType = std::variant<std::string, bool, long long>;

inline std::ostream& operator<<(std::ostream& os, const ValueType& value) {
    std::visit([&os](auto&& arg) { os << arg; }, value);
    return os;
}

inline bool operator==(const std::shared_ptr<Expression>& expression, const ValueType& value) {
    if (expression->type() == Statement::Type::Identifier) {
        auto identifier = std::dynamic_pointer_cast<Identifier>(expression);
        return identifier && identifier->tokenLiteral() == std::get<std::string>(value);
    } else if (expression->type() == Statement::Type::Boolean) {
        auto boolean = std::dynamic_pointer_cast<Boolean>(expression);
        return boolean && boolean->value() == std::get<bool>(value);
    } else if (expression->type() == Statement::Type::IntegerLiteral) {
        auto integer_literal = std::dynamic_pointer_cast<IntegerLiteral>(expression);
        return integer_literal && integer_literal->tokenLiteral() == std::to_string(std::get<long long>(value));
    }
    return false;
}

#define TEST_LET_STATEMENT(statement, name_, value_, str_)                     \
    {                                                                          \
        auto let_statement = reinterpret_cast<LetStatement*>(statement.get()); \
        EXPECT_EQ(let_statement->tokenLiteral(), "let");                       \
        EXPECT_EQ(let_statement->name()->tokenLiteral(), name_);               \
        EXPECT_EQ(let_statement->value(), value_);                             \
        EXPECT_EQ(let_statement->toString(), str_);                            \
    }

#define TEST_RETURN_STATEMENT(statement, value_, str_)                               \
    {                                                                                \
        auto return_statement = reinterpret_cast<ReturnStatement*>(statement.get()); \
        EXPECT_EQ(return_statement->tokenLiteral(), "return");                       \
        EXPECT_EQ(return_statement->value(), value_);                                \
        EXPECT_EQ(return_statement->toString(), str_);                               \
    }

#define TEST_IDENTIFIER(expression, literal)                                            \
    {                                                                                   \
        const auto& identifier = reinterpret_cast<const Identifier*>(expression.get()); \
        EXPECT_EQ(identifier->tokenLiteral(), literal);                                 \
        EXPECT_EQ(identifier->toString(), literal);                                     \
    }

#define TEST_BOOLEAN(expression, value_, literal)                    \
    {                                                                \
        auto boolean = reinterpret_cast<Boolean*>(expression.get()); \
        EXPECT_EQ(boolean->value(), value_);                         \
        EXPECT_EQ(boolean->toString(), literal);                     \
    }

#define TEST_INTEGER_LITERAL(expression, literal)                                   \
    {                                                                               \
        auto integer_literal = reinterpret_cast<IntegerLiteral*>(expression.get()); \
        EXPECT_EQ(integer_literal->tokenLiteral(), literal);                        \
        EXPECT_EQ(integer_literal->toString(), literal);                            \
    }

#define TEST_STRING_LITERAL(expression, literal)                                  \
    {                                                                             \
        auto string_literal = reinterpret_cast<StringLiteral*>(expression.get()); \
        EXPECT_EQ(string_literal->tokenLiteral(), literal);                       \
        EXPECT_EQ(string_literal->toString(), literal);                           \
    }

#define TEST_PREFIX_EXPRESSION(expression, operator_str, right_)                               \
    {                                                                                          \
        auto prefix_expression = reinterpret_cast<PrefixExpression*>(expression.get());        \
        EXPECT_EQ(prefix_expression->tokenLiteral(), operator_str);                            \
        EXPECT_EQ(prefix_expression->right(), right_);                                         \
        EXPECT_EQ(prefix_expression->toString(), fmt::format("({}{})", operator_str, right_)); \
    }

#define TEST_INFIX_EXPRESSION(expression, left_, operator_str, right_)                                   \
    {                                                                                                    \
        auto infix_expression = reinterpret_cast<InfixExpression*>(expression.get());                    \
        EXPECT_EQ(infix_expression->left(), left_);                                                      \
        EXPECT_EQ(infix_expression->tokenLiteral(), operator_str);                                       \
        EXPECT_EQ(infix_expression->right(), right_);                                                    \
        EXPECT_EQ(infix_expression->toString(), fmt::format("({} {} {})", left_, operator_str, right_)); \
    }

TEST(ParserTest, LetStatementTest) {
    struct Input {
        std::string input;
        std::string identifier;
        ValueType value;
    };

    Input inputs[] = {
        {"let x = 5;", "x", 5},
        {"let y = true;", "y", true},
        {"let foobar = y;", "foobar", "y"},
    };

    for (const auto& input : inputs) {
        auto parser = Parser::New(Lexer::New(input.input));
        auto program = parser->parseProgram();

        ASSERT_TRUE(program && parser->errors().empty());
        EXPECT_EQ(program->statements().size(), 1);
        const auto& statement = program->statements()[0];
        EXPECT_EQ(statement->type(), Statement::Type::LetStatement);

        TEST_LET_STATEMENT(statement, input.identifier, input.value, input.input);
    }
}

TEST(ParserTest, ReturnStatementTest) {
    struct Input {
        std::string input;
        ValueType value;
    };

    Input inputs[] = {
        {"return 5;", 5},
        {"return true;", true},
        {"return foobar;", "foobar"},
    };

    for (const auto& input : inputs) {
        auto parser = Parser::New(Lexer::New(input.input));
        auto program = parser->parseProgram();

        ASSERT_TRUE(program && parser->errors().empty());
        EXPECT_EQ(program->statements().size(), 1);
        const auto& statement = program->statements()[0];
        EXPECT_EQ(statement->type(), Statement::Type::ReturnStatement);

        TEST_RETURN_STATEMENT(statement, input.value, input.input);
    }
}

TEST(ParserTest, IdentifierTest) {
    std::string input = "foobar;";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::Identifier);

    TEST_IDENTIFIER(expression, "foobar");
}

TEST(ParserTest, BooleanTest) {
    struct Input {
        std::string input;
        bool value;
    };

    Input inputs[] = {
        {"true", true},
        {"false", false},
    };

    for (const auto& input : inputs) {
        auto parser = Parser::New(Lexer::New(input.input));
        auto program = parser->parseProgram();

        ASSERT_TRUE(program && parser->errors().empty());
        EXPECT_EQ(program->statements().size(), 1);
        const auto& statement = program->statements()[0];
        EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
        const auto& expression = reinterpret_cast<ExpressionStatement*>(statement.get())->expression();
        EXPECT_EQ(expression->type(), Expression::Type::Boolean);

        TEST_BOOLEAN(expression, input.value, input.input);
    }
}

TEST(ParserTest, IntegerLiteralTest) {
    std::string input = "5;";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<const ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::IntegerLiteral);

    TEST_INTEGER_LITERAL(expression, "5");
}

TEST(ParserTest, StringLiteralTest) {
    std::string input = "\"hello world\";";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<const ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::StringLiteral);

    TEST_STRING_LITERAL(expression, "hello world");
}

TEST(ParserTest, ArrayLiteralTest) {
    std::string input = "[1, 2 * 2, 3 + 3]";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<const ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::ArrayLiteral);

    auto array_literal = reinterpret_cast<ArrayLiteral*>(expression.get());
    EXPECT_EQ(array_literal->elements().size(), 3);
    TEST_INTEGER_LITERAL(array_literal->elements()[0], "1");
    TEST_INFIX_EXPRESSION(array_literal->elements()[1], 2, "*", 2);
    TEST_INFIX_EXPRESSION(array_literal->elements()[2], 3, "+", 3);
}

TEST(ParserTest, HashLiteralTest) {
    std::string input = "{\"one\": 1, \"two\": 2, \"three\": 3}";
    std::map<std::string_view, int64_t> expected{{"one", 1}, {"two", 2}, {"three", 3}};

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<const ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::HashLiteral);

    auto hash_literal = reinterpret_cast<HashLiteral*>(expression.get());
    EXPECT_EQ(hash_literal->pairs().size(), 3);
    for (const auto& [key, value] : hash_literal->pairs()) {
        auto key_str = std::dynamic_pointer_cast<StringLiteral>(key);
        ASSERT_TRUE(key_str);
        EXPECT_TRUE(expected.count(key_str->tokenLiteral()) > 0);
        TEST_INTEGER_LITERAL(value, std::to_string(expected[key_str->tokenLiteral()]));
    }
}

TEST(ParserTest, EmptyHashLiteralTest) {
    std::string input = "{}";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<const ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::HashLiteral);

    auto hash_literal = reinterpret_cast<HashLiteral*>(expression.get());
    EXPECT_TRUE(hash_literal->pairs().empty());
}

TEST(ParserTest, HashLiteralWithExpressionTest) {
    std::string input = "{\"one\": 0+1, \"two\": 10-8, \"three\": 15/5}";
    using func = std::function<void(std::shared_ptr<Expression>)>;
    std::map<std::string_view, func> expected{
        {"one", [](std::shared_ptr<Expression> e) { TEST_INFIX_EXPRESSION(e, 0, "+", 1); }},
        {"two", [](std::shared_ptr<Expression> e) { TEST_INFIX_EXPRESSION(e, 10, "-", 8); }},
        {"three", [](std::shared_ptr<Expression> e) { TEST_INFIX_EXPRESSION(e, 15, "/", 5); }}};

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<const ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::HashLiteral);

    auto hash_literal = reinterpret_cast<HashLiteral*>(expression.get());
    EXPECT_EQ(hash_literal->pairs().size(), 3);
    for (const auto& [key, value] : hash_literal->pairs()) {
        auto key_str = std::dynamic_pointer_cast<StringLiteral>(key);
        ASSERT_TRUE(key_str);
        EXPECT_TRUE(expected.count(key_str->tokenLiteral()) > 0);
        expected[key_str->tokenLiteral()](value);
    }
}

TEST(ParserTest, IndexExpressionTest) {
    std::string input = "myArray[1 + 1]";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<const ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::IndexExpression);

    auto index_expression = reinterpret_cast<IndexExpression*>(expression.get());
    EXPECT_EQ(index_expression->left(), "myArray");
    TEST_INFIX_EXPRESSION(index_expression->index(), 1, "+", 1);
}

TEST(ParserTest, PrefixExpressionTest) {
    struct Input {
        std::string input;
        std::string operator_str;
        ValueType right;
    };

    Input inputs[] = {
        {"!5;", "!", 5},       {"-15;", "-", 15},       {"!foobar;", "!", "foobar"}, {"-foobar;", "-", "foobar"},
        {"!true;", "!", true}, {"!false;", "!", false},
    };

    for (const auto& input : inputs) {
        auto parser = Parser::New(Lexer::New(input.input));
        auto program = parser->parseProgram();

        ASSERT_TRUE(program && parser->errors().empty());
        EXPECT_EQ(program->statements().size(), 1);
        const auto& statement = program->statements()[0];
        EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
        const auto& expression = reinterpret_cast<const ExpressionStatement*>(statement.get())->expression();
        EXPECT_EQ(expression->type(), Expression::Type::PrefixExpression);

        TEST_PREFIX_EXPRESSION(expression, input.operator_str, input.right);
    }
}

TEST(ParserTest, InfixExpressionTest) {
    struct Input {
        std::string input;
        ValueType left;
        std::string operator_str;
        ValueType right;
    };

    Input inputs[] = {
        {"5 + 5;", 5, "+", 5},
        {"5 - 5;", 5, "-", 5},
        {"5 * 5;", 5, "*", 5},
        {"5 / 5;", 5, "/", 5},
        {"5 > 5;", 5, ">", 5},
        {"5 < 5;", 5, "<", 5},
        {"5 == 5;", 5, "==", 5},
        {"5 != 5;", 5, "!=", 5},
        {"foobar + barfoo;", "foobar", "+", "barfoo"},
        {"foobar - barfoo;", "foobar", "-", "barfoo"},
        {"foobar * barfoo;", "foobar", "*", "barfoo"},
        {"foobar / barfoo;", "foobar", "/", "barfoo"},
        {"foobar > barfoo;", "foobar", ">", "barfoo"},
        {"foobar < barfoo;", "foobar", "<", "barfoo"},
        {"foobar == barfoo;", "foobar", "==", "barfoo"},
        {"foobar != barfoo;", "foobar", "!=", "barfoo"},
        {"true == true", true, "==", true},
        {"true != false", true, "!=", false},
        {"false == false", false, "==", false},
    };

    for (const auto& input : inputs) {
        auto parser = Parser::New(Lexer::New(input.input));
        auto program = parser->parseProgram();

        EXPECT_TRUE(program && parser->errors().empty());
        EXPECT_EQ(program->statements().size(), 1);
        const auto& statement = program->statements()[0];
        EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
        const auto& expression = reinterpret_cast<const ExpressionStatement*>(statement.get())->expression();

        TEST_INFIX_EXPRESSION(expression, input.left, input.operator_str, input.right);
    }
}

TEST(ParserTest, OperatorPrecedenceTest) {
    struct Input {
        std::string input;
        std::string expected;
    };

    Input inputs[]{
        {
            "-a * b",
            "((-a) * b)",
        },
        {
            "!-a",
            "(!(-a))",
        },
        {
            "a + b + c",
            "((a + b) + c)",
        },
        {
            "a + b - c",
            "((a + b) - c)",
        },
        {
            "a * b * c",
            "((a * b) * c)",
        },
        {
            "a * b / c",
            "((a * b) / c)",
        },
        {
            "a + b / c",
            "(a + (b / c))",
        },
        {
            "a + b * c + d / e - f",
            "(((a + (b * c)) + (d / e)) - f)",
        },
        {
            "3 + 4; -5 * 5",
            "(3 + 4)((-5) * 5)",
        },
        {
            "5 > 4 == 3 < 4",
            "((5 > 4) == (3 < 4))",
        },
        {
            "5 < 4 != 3 > 4",
            "((5 < 4) != (3 > 4))",
        },
        {
            "3 + 4 * 5 == 3 * 1 + 4 * 5",
            "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
        },
        {
            "true",
            "true",
        },
        {
            "false",
            "false",
        },
        {
            "3 > 5 == false",
            "((3 > 5) == false)",
        },
        {
            "3 < 5 == true",
            "((3 < 5) == true)",
        },
        {
            "1 + (2 + 3) + 4",
            "((1 + (2 + 3)) + 4)",
        },
        {
            "(5 + 5) * 2",
            "((5 + 5) * 2)",
        },
        {
            "2 / (5 + 5)",
            "(2 / (5 + 5))",
        },
        {
            "(5 + 5) * 2 * (5 + 5)",
            "(((5 + 5) * 2) * (5 + 5))",
        },
        {
            "-(5 + 5)",
            "(-(5 + 5))",
        },
        {
            "!(true == true)",
            "(!(true == true))",
        },
        {
            "a + add(b * c) + d",
            "((a + add((b * c))) + d)",
        },
        {
            "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
            "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))",
        },
        {
            "add(a + b + c * d / f + g)",
            "add((((a + b) + ((c * d) / f)) + g))",
        },
    };

    for (const auto& input : inputs) {
        auto parser = Parser::New(Lexer::New(input.input));
        auto program = parser->parseProgram();

        ASSERT_TRUE(program && parser->errors().empty());
        EXPECT_EQ(program->toString(), input.expected);
    }
}

TEST(ParserTest, IfExpressionTest) {
    std::string input = "if (x < y) { x }";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::IfExpression);

    // condition
    const auto& if_expression = reinterpret_cast<IfExpression*>(expression.get());
    EXPECT_EQ(if_expression->tokenLiteral(), "if");
    TEST_INFIX_EXPRESSION(if_expression->condition(), "x", "<", "y");

    // consequence
    EXPECT_EQ(if_expression->consequence()->statements().size(), 1);
    const auto& consequence = if_expression->consequence()->statements()[0];
    EXPECT_EQ(consequence->type(), Statement::Type::ExpressionStatement);
    const auto& consequence_expression = reinterpret_cast<ExpressionStatement*>(consequence.get())->expression();
    EXPECT_EQ(consequence_expression->type(), Expression::Type::Identifier);
    TEST_IDENTIFIER(consequence_expression, "x");

    // alternative
    EXPECT_EQ(if_expression->alternative(), nullptr);

    EXPECT_EQ(if_expression->toString(), input);
}

TEST(ParserTest, IfElseExpressionTest) {
    std::string input = "if (x < y) { x } else { y }";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::IfExpression);

    // condition
    const auto& if_expression = reinterpret_cast<IfExpression*>(expression.get());
    EXPECT_EQ(if_expression->tokenLiteral(), "if");
    TEST_INFIX_EXPRESSION(if_expression->condition(), "x", "<", "y");

    // consequence
    EXPECT_EQ(if_expression->consequence()->statements().size(), 1);
    const auto& consequence = if_expression->consequence()->statements()[0];
    EXPECT_EQ(consequence->type(), Statement::Type::ExpressionStatement);
    const auto& consequence_expression = reinterpret_cast<ExpressionStatement*>(consequence.get())->expression();
    EXPECT_EQ(consequence_expression->type(), Expression::Type::Identifier);
    TEST_IDENTIFIER(consequence_expression, "x");

    // alternative
    ASSERT_TRUE(if_expression->alternative());
    EXPECT_EQ(if_expression->alternative()->statements().size(), 1);
    const auto& alternative = if_expression->alternative()->statements()[0];
    EXPECT_EQ(alternative->type(), Statement::Type::ExpressionStatement);
    const auto& alternative_expression = reinterpret_cast<ExpressionStatement*>(alternative.get())->expression();
    EXPECT_EQ(alternative_expression->type(), Expression::Type::Identifier);
    TEST_IDENTIFIER(alternative_expression, "y");

    EXPECT_EQ(if_expression->toString(), input);
}

TEST(ParserTest, FunctionLiteralTest) {
    std::string input = "fn(x, y) { x + y; }";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::FunctionLiteral);

    // function literal
    const auto& function_literal = reinterpret_cast<FunctionLiteral*>(expression.get());
    EXPECT_EQ(function_literal->tokenLiteral(), "fn");

    // parameters
    EXPECT_EQ(function_literal->parameters().size(), 2);
    TEST_IDENTIFIER(function_literal->parameters()[0], "x");
    TEST_IDENTIFIER(function_literal->parameters()[1], "y");

    // body
    EXPECT_EQ(function_literal->body()->statements().size(), 1);
    const auto& body_statement = function_literal->body()->statements()[0];
    EXPECT_EQ(body_statement->type(), Statement::Type::ExpressionStatement);
    const auto& body_expression = reinterpret_cast<ExpressionStatement*>(body_statement.get())->expression();
    TEST_INFIX_EXPRESSION(body_expression, "x", "+", "y");

    EXPECT_EQ(function_literal->toString(), "fn(x, y) { (x + y) }");
}

TEST(ParserTest, FunctionParameterTest) {
    struct Input {
        std::string input;
        std::vector<std::string> params;
    };

    Input inputs[] = {
        {"fn() {};", {}},
        {"fn(x) {};", {"x"}},
        {"fn(x, y, z) {};", {"x", "y", "z"}},
    };

    for (const auto& input : inputs) {
        auto parser = Parser::New(Lexer::New(input.input));
        auto program = parser->parseProgram();

        ASSERT_TRUE(program && parser->errors().empty()) << parser->errorsToString();
        EXPECT_EQ(program->statements().size(), 1);
        const auto& statement = program->statements()[0];
        EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
        const auto& expression = reinterpret_cast<ExpressionStatement*>(statement.get())->expression();
        EXPECT_EQ(expression->type(), Expression::Type::FunctionLiteral);

        const auto& function_literal = reinterpret_cast<FunctionLiteral*>(expression.get());
        EXPECT_EQ(function_literal->parameters().size(), input.params.size());

        for (size_t i = 0; i < input.params.size(); ++i) {
            TEST_IDENTIFIER(function_literal->parameters()[i], input.params[i]);
        }
    }
}

TEST(ParserTest, CallExpressionTest) {
    std::string input = "add(1, 2 * 3, 4 + 5);";

    auto parser = Parser::New(Lexer::New(input));
    auto program = parser->parseProgram();

    ASSERT_TRUE(program && parser->errors().empty());
    EXPECT_EQ(program->statements().size(), 1);
    const auto& statement = program->statements()[0];
    EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
    const auto& expression = reinterpret_cast<ExpressionStatement*>(statement.get())->expression();
    EXPECT_EQ(expression->type(), Expression::Type::CallExpression);

    // function literal
    const auto& call_expression = reinterpret_cast<CallExpression*>(expression.get());
    EXPECT_EQ(call_expression->tokenLiteral(), "(");

    // function
    EXPECT_EQ(call_expression->function()->type(), Expression::Type::Identifier);
    const auto& function = reinterpret_cast<Identifier*>(call_expression->function().get());
    EXPECT_EQ(function->tokenLiteral(), "add");

    // arguments
    EXPECT_EQ(call_expression->arguments().size(), 3);
    TEST_INTEGER_LITERAL(call_expression->arguments()[0], "1");
    TEST_INFIX_EXPRESSION(call_expression->arguments()[1], 2, "*", 3);
    TEST_INFIX_EXPRESSION(call_expression->arguments()[2], 4, "+", 5);

    EXPECT_EQ(call_expression->toString(), "add(1, (2 * 3), (4 + 5))");
}

TEST(ParserTest, CallExpressionParameterTest) {
    struct Input {
        std::string input;
        std::string identifier;
        std::vector<std::string> args;
    };

    Input inputs[] = {
        {
            "add();",
            "add",
            {},
        },
        {
            "add(1);",
            "add",
            {"1"},
        },
        {
            "add(1, 2 * 3, 4 + 5);",
            "add",
            {"1", "(2 * 3)", "(4 + 5)"},
        },
    };

    for (const auto& input : inputs) {
        auto parser = Parser::New(Lexer::New(input.input));
        auto program = parser->parseProgram();

        ASSERT_TRUE(program && parser->errors().empty());
        EXPECT_EQ(program->statements().size(), 1);
        const auto& statement = program->statements()[0];
        EXPECT_EQ(statement->type(), Statement::Type::ExpressionStatement);
        const auto& expression = reinterpret_cast<ExpressionStatement*>(statement.get())->expression();
        EXPECT_EQ(expression->type(), Expression::Type::CallExpression);

        // function literal
        const auto& call_expression = reinterpret_cast<CallExpression*>(expression.get());
        EXPECT_EQ(call_expression->tokenLiteral(), "(");

        // function
        EXPECT_EQ(call_expression->function()->type(), Expression::Type::Identifier);
        TEST_IDENTIFIER(call_expression->function(), input.identifier);

        // arguments
        EXPECT_EQ(call_expression->arguments().size(), input.args.size());
        for (size_t i = 0; i < call_expression->arguments().size(); i++) {
            EXPECT_EQ(call_expression->arguments()[i]->toString(), input.args[i]);
        }
    }
}

}  // namespace monkey
}  // namespace pyc
