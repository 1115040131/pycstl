#include <fmt/printf.h>
#include <gtest/gtest.h>

#include "test/proto/book.pb.h"

template <>
struct fmt::formatter<network::Book> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const network::Book& book, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{{name: {}, pages: {}, price: {}}}", book.name(), book.pages(), book.price());
    }
};

TEST(ProtobufTest, BookTest) {
    network::Book book;
    book.set_name("CPP programing");
    book.set_pages(100);
    book.set_price(200);

    std::string book_str;
    book.SerializeToString(&book_str);
    fmt::println("Serialize str is {}", book_str);

    network::Book book2;
    book2.ParseFromString(book_str);
    fmt::println("Book: {}", book2);
}