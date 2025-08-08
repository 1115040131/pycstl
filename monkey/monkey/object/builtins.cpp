#include "monkey/object/builtins.h"

#include <unordered_map>

namespace pyc {
namespace monkey {

#define DEF_BUILTIN(name) \
    inline static std::shared_ptr<Object> Builtin_##name(const std::vector<std::shared_ptr<Object>>& args)

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
    return std::make_shared<Error>(fmt::format("argument to `len` not supported, got {}", args[0]->typeStr()));
}

DEF_BUILTIN(puts) {
    for (const auto& arg : args) {
        fmt::println("{}", arg->inspect());
    }
    return kNullObj;  // Puts does not return a value, so we return null
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
    return std::make_shared<Error>(fmt::format("argument to `first` must be ARRAY, got {}", args[0]->typeStr()));
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
    return std::make_shared<Error>(fmt::format("argument to `last` must be ARRAY, got {}", args[0]->typeStr()));
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
    return std::make_shared<Error>(fmt::format("argument to `rest` must be ARRAY, got {}", args[0]->typeStr()));
}

DEF_BUILTIN(push) {
    if (args.size() != 2) {
        return std::make_shared<Error>(fmt::format("wrong number of arguments. got={}, want=2", args.size()));
    }
    if (auto array = std::dynamic_pointer_cast<Array>(args[0])) {
        auto elements = array->elements();
        elements.push_back(args[1]);
        return std::make_shared<Array>(std::move(elements));  // Return a new array with the element pushed
    }
    return std::make_shared<Error>(fmt::format("argument to `push` must be ARRAY, got {}", args[0]->typeStr()));
}

int fibonacci(int num) {
    if (num == 0) {
        return 0;
    } else if (num == 1) {
        return 1;
    } else {
        return fibonacci(num - 1) + fibonacci(num - 2);
    }
}

DEF_BUILTIN(fibonacci) {
    if (args.size() != 1) {
        return std::make_shared<Error>(fmt::format("wrong number of arguments. got={}, want=1", args.size()));
    }
    if (auto integer = std::dynamic_pointer_cast<Integer>(args[0])) {
        if (integer->value() < 0) {
            return std::make_shared<Error>(
                fmt::format("argument to `fibonacci` can not be negative, got {}", integer->value()));
        }
        return std::make_shared<Integer>(fibonacci(integer->value()));
    }
    return std::make_shared<Error>(
        fmt::format("argument to `fibonacci` must be INTEGER, got {}", args[0]->typeStr()));
}

#define ALL_BUILTINS                                                                               \
    ADD_BUILTIN(len), ADD_BUILTIN(puts), ADD_BUILTIN(first), ADD_BUILTIN(last), ADD_BUILTIN(rest), \
        ADD_BUILTIN(push), ADD_BUILTIN(fibonacci)

inline static std::vector<BuiltinWithName> BuiltinList{ALL_BUILTINS};

inline static std::unordered_map<std::string_view, std::shared_ptr<Builtin>> BuiltinMap{ALL_BUILTINS};

const std::vector<BuiltinWithName>& GetBuiltinList() { return BuiltinList; }

std::shared_ptr<Builtin> GetBuiltinByName(std::string_view name) {
    auto iter = BuiltinMap.find(name);
    if (iter != BuiltinMap.end()) {
        return iter->second;
    }
    return nullptr;
}

}  // namespace monkey
}  // namespace pyc