#include <unordered_map>

#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

class Environment {
public:
    static std::shared_ptr<Environment> New() { return std::make_shared<Environment>(); }

    static std::shared_ptr<Environment> NewEnclosed(std::shared_ptr<Environment> outer) {
        return std::make_shared<Environment>(std::move(outer));
    }

    Environment() = default;
    explicit Environment(std::shared_ptr<Environment> outer) : outer_(std::move(outer)) {}

    void set(std::string_view name, std::shared_ptr<Object> value) {
        store_[std::string(name)] = std::move(value);
    }

    std::shared_ptr<Object> get(std::string_view name) {
        auto it = store_.find(std::string(name));
        if (it != store_.end()) {
            return it->second;
        }
        if (outer_) {
            return outer_->get(name);
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> store_;
    std::shared_ptr<Environment> outer_ = nullptr;
};

}  // namespace monkey
}  // namespace pyc