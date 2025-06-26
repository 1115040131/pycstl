#pragma once

#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

std::shared_ptr<Builtin> GetBuiltin(std::string_view name);

}  // namespace monkey
}  // namespace pyc