#include <iostream>

template <typename T>
struct Base {
    T value_;
};

template <typename T>
struct Derived : Base<T> {
    void print() {
        std::cout << this->value_ << std::endl;
    }
};

int main() {
    return 0;
}