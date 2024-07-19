#include <iostream>
#include <sstream>

#include "common/noncopyable.h"

namespace pyc {

/// @breif: RAII 类用于管理 cin 的缓冲区重定向
class CinRedirect : public Noncopyable {
public:
    // 在构造函数中进行重定向
    explicit CinRedirect(std::istream& new_cin) : original_cin_buffer_(std::cin.rdbuf(new_cin.rdbuf())) {}

    // 在析构函数中恢复原始缓冲区
    ~CinRedirect() { std::cin.rdbuf(original_cin_buffer_); }

private:
    std::streambuf* original_cin_buffer_;
};

}  // namespace pyc
