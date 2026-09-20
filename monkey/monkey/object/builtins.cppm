module;

#include <memory>
#include <string>
#include <string_view>
#include <vector>

export module monkey.object.builtins;

export import monkey.object;

export namespace pyc::monkey {

struct BuiltinWithName {
    std::string name;
    std::shared_ptr<Builtin> builtin;
};

const std::vector<BuiltinWithName>& GetBuiltinList();

std::shared_ptr<Builtin> GetBuiltinByName(std::string_view name);

}  // namespace pyc::monkey
