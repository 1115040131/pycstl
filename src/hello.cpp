#include <format>
#include <iostream>

void printHello() {
    std::cout << std::format("{} {}!", "hello", "world");
}