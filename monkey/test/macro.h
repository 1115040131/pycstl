#pragma once

#define TEST_NULL_OBJECT(object, input)                                                                     \
    {                                                                                                       \
        auto null = std::dynamic_pointer_cast<Null>(object);                                                \
        ASSERT_TRUE(null != nullptr) << "Input: " << input << "\nExpected Null, got " << object->typeStr(); \
    }

#define TEST_INTEGER_OBJECT(object, expected, input)                                                              \
    {                                                                                                             \
        auto integer = std::dynamic_pointer_cast<Integer>(object);                                                \
        ASSERT_TRUE(integer != nullptr) << "Input: " << input << "\nExpected Integer, got " << object->typeStr(); \
        EXPECT_EQ(integer->value(), expected) << "Input: " << input;                                              \
    }

#define TEST_BOOLEAN_OBJECT(object, expected, input)                                        \
    {                                                                                       \
        auto boolean = std::dynamic_pointer_cast<BooleanObject>(object);                    \
        ASSERT_TRUE(boolean != nullptr)                                                     \
            << "Input: " << input << "\nExpected BooleanObject, got " << object->typeStr(); \
        EXPECT_EQ(boolean->value(), expected) << "Input: " << input;                        \
    }

#define TEST_STRING_OBJECT(object, expected, input)                                                          \
    {                                                                                                        \
        auto str = std::dynamic_pointer_cast<String>(object);                                                \
        ASSERT_TRUE(str != nullptr) << "Input: " << input << "\nExpected String, got " << object->typeStr(); \
        EXPECT_EQ(str->value(), expected) << "Input: " << input;                                             \
    }

#define TEST_COMPILED_FUNCTION(object, expected, input)                                    \
    {                                                                                      \
        auto compiled_function = std::dynamic_pointer_cast<CompiledFunction>(object);      \
        auto actual = compiled_function->instructions();                                   \
        auto concated = concateInstructions(expected);                                     \
        ASSERT_EQ(concated.size(), actual.size()) << "Input: " << input << "\nconcated:\n" \
                                                  << toString(concated) << "actual:\n"     \
                                                  << toString(actual);                     \
        for (size_t i = 0; i < concated.size(); i++) {                                     \
            EXPECT_EQ(concated[i], actual[i]) << "Input: " << input << "\nconcated:\n"     \
                                              << toString(concated) << "actual:\n"         \
                                              << toString(actual);                         \
        }                                                                                  \
    }

#define TEST_EXPECTED_OBJECT(object, expected, input)                                             \
    {                                                                                             \
        if (std::holds_alternative<int>(expected)) {                                              \
            TEST_INTEGER_OBJECT(object, std::get<int>(expected), input);                          \
        } else if (std::holds_alternative<bool>(expected)) {                                      \
            TEST_BOOLEAN_OBJECT(object, std::get<bool>(expected), input);                         \
        } else if (std::holds_alternative<std::string>(expected)) {                               \
            auto expected_str = std::get<std::string>(expected);                                  \
            if (auto array = std::dynamic_pointer_cast<Array>(object)) {                          \
                EXPECT_EQ(array->inspect(), expected_str) << "Input: " << input;                  \
            } else if (auto hash = std::dynamic_pointer_cast<Hash>(object)) {                     \
                EXPECT_EQ(hash->inspect(), expected_str) << "Input: " << input;                   \
            } else if (auto error = std::dynamic_pointer_cast<Error>(object)) {                   \
                EXPECT_EQ(error->inspect(), expected_str) << "Input: " << input;                  \
            } else {                                                                              \
                TEST_STRING_OBJECT(object, expected_str, input);                                  \
            }                                                                                     \
        } else if (std::holds_alternative<std::vector<Instructions>>(expected)) {                 \
            TEST_COMPILED_FUNCTION(object, std::get<std::vector<Instructions>>(expected), input); \
        } else {                                                                                  \
            TEST_NULL_OBJECT(object, input);                                                      \
        }                                                                                         \
    }
