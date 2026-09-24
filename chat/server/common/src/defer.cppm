module;

#include <utility>

export module chat.server.common.defer;

export namespace pyc::chat {

template <typename Deleter>
class Defer {
public:
    Defer(Deleter&& deleter) : deleter_(std::move(deleter)) {}

    ~Defer() { deleter_(); }

private:
    Deleter deleter_;
};

}  // namespace pyc::chat
