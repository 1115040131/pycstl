#include "monkey/evaluator/builtins.h"

#include <unordered_map>

namespace pyc {
namespace monkey {

#define DEF_BUILTIN(name) \
    static std::shared_ptr<Object> Builtin_##name(const std::vector<std::shared_ptr<Object>>& args)

#define ADD_BUILTIN(name) {#name, std::make_shared<Builtin>(&Builtin_##name)}

DEF_BUILTIN(len) {
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

DEF_BUILTIN(first) {
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

DEF_BUILTIN(last) {
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

DEF_BUILTIN(rest) {
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

DEF_BUILTIN(push) {
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

DEF_BUILTIN(puts) {
    for (const auto& arg : args) {
        fmt::println("{}", arg->inspect());
    }
    return kNullObj;  // Puts does not return a value, so we return null
}

std::shared_ptr<Builtin> GetBuiltin(std::string_view name) {
    static std::unordered_map<std::string_view, std::shared_ptr<Builtin>> builtins{
        ADD_BUILTIN(len),  ADD_BUILTIN(first), ADD_BUILTIN(last),
        ADD_BUILTIN(rest), ADD_BUILTIN(push),  ADD_BUILTIN(puts),
    };

    auto iter = builtins.find(name);
    if (iter != builtins.end()) {
        return iter->second;
    }
    return nullptr;
}

}  // namespace monkey
}  // namespace pyc