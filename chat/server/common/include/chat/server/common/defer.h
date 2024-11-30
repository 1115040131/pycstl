#pragma once

namespace pyc {
namespace chat {

template <typename Deleter>
class Defer {
public:
    Defer(Deleter&& deleter) : deleter_(std::move(deleter)) {}

    ~Defer() { deleter_(); }

private:
    Deleter deleter_;
};

}  // namespace chat
}  // namespace pyc
