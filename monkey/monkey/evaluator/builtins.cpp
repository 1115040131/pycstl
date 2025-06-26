#include "monkey/evaluator/builtins.h"

#include <unordered_map>

namespace pyc {
namespace monkey {

std::shared_ptr<Object> lenFunc(const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        return std::make_shared<Error>(fmt::format("wrong number of arguments. got={}, want=1", args.size()));
    }
    if (auto str = std::dynamic_pointer_cast<String>(args[0])) {
        return std::make_shared<Integer>(str->value().size());
    }
    return std::make_shared<Error>(fmt::format("argument to 'len' not supported, got {}", args[0]->typeStr()));
}

std::shared_ptr<Builtin> GetBuiltin(std::string_view name) {
    static std::unordered_map<std::string_view, std::shared_ptr<Builtin>> builtins{
        {"len", std::make_shared<Builtin>(&lenFunc)},
    };

    auto iter = builtins.find(name);
    if (iter != builtins.end()) {
        return iter->second;
    }
    return nullptr;
}

}  // namespace monkey
}  // namespace pyc