#pragma once

#include <vector>

#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

struct BuiltinWithName {
    std::string Name;
    std::shared_ptr<Builtin> Builtin;
};

const std::vector<BuiltinWithName>& GetBuiltinList();

std::shared_ptr<Builtin> GetBuiltinByName(std::string_view name);

}  // namespace monkey
}  // namespace pyc