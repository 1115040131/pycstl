#include "monkey/evaluator/builtins.h"

#include <unordered_map>

namespace pyc {
namespace monkey {

std::shared_ptr<Object> Builtin_len(const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        return std::make_shared<Error>(fmt::format("wrong number of arguments. got={}, want=1", args.size()));
    }
    if (auto str = std::dynamic_pointer_cast<String>(args[0])) {
        return std::make_shared<Integer>(str->value().size());
    } else if (auto array = std::dynamic_pointer_cast<Array>(args[0])) {
        return std::make_shared<Integer>(array->elements().size());
    }
    return std::make_shared<Error>(fmt::format("argument to 'len' not supported, got {}", args[0]->typeStr()));
}

std::shared_ptr<Object> Builtin_first(const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        return std::make_shared<Error>(fmt::format("wrong number of arguments. got={}, want=1", args.size()));
    }
    if (auto array = std::dynamic_pointer_cast<Array>(args[0])) {
        if (array->elements().empty()) {
            return kNullObj;
        }
        return array->elements().front();
    }
    return std::make_shared<Error>(fmt::format("argument to 'first' must be ARRAY, got {}", args[0]->typeStr()));
}

std::shared_ptr<Object> Builtin_last(const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        return std::make_shared<Error>(fmt::format("wrong number of arguments. got={}, want=1", args.size()));
    }
    if (auto array = std::dynamic_pointer_cast<Array>(args[0])) {
        if (array->elements().empty()) {
            return kNullObj;
        }
        return array->elements().back();
    }
    return std::make_shared<Error>(fmt::format("argument to 'last' must be ARRAY, got {}", args[0]->typeStr()));
}

std::shared_ptr<Object> Builtin_rest(const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        return std::make_shared<Error>(fmt::format("wrong number of arguments. got={}, want=1", args.size()));
    }
    if (auto array = std::dynamic_pointer_cast<Array>(args[0])) {
        if (array->elements().empty()) {
            return kNullObj;
        }
        return std::make_shared<Array>(
            std::vector<std::shared_ptr<Object>>(array->elements().begin() + 1, array->elements().end()));
    }
    return std::make_shared<Error>(fmt::format("argument to 'rest' must be ARRAY, got {}", args[0]->typeStr()));
}

std::shared_ptr<Object> Builtin_push(const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 2) {
        return std::make_shared<Error>(fmt::format("wrong number of arguments. got={}, want=1", args.size()));
    }
    if (auto array = std::dynamic_pointer_cast<Array>(args[0])) {
        auto elements = array->elements();
        elements.push_back(args[1]);
        return std::make_shared<Array>(std::move(elements));  // Return a new array with the element pushed
    }
    return std::make_shared<Error>(fmt::format("argument to 'push' must be ARRAY, got {}", args[0]->typeStr()));
}

std::shared_ptr<Builtin> GetBuiltin(std::string_view name) {
    static std::unordered_map<std::string_view, std::shared_ptr<Builtin>> builtins{
        {"len", std::make_shared<Builtin>(&Builtin_len)},   {"first", std::make_shared<Builtin>(&Builtin_first)},
        {"last", std::make_shared<Builtin>(&Builtin_last)}, {"rest", std::make_shared<Builtin>(&Builtin_rest)},
        {"push", std::make_shared<Builtin>(&Builtin_push)},
    };

    auto iter = builtins.find(name);
    if (iter != builtins.end()) {
        return iter->second;
    }
    return nullptr;
}

}  // namespace monkey
}  // namespace pyc