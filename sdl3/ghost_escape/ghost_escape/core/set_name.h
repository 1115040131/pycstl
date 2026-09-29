#pragma once

#ifdef DEBUG_MODE

#include <fmt/format.h>

#include "ghost_escape/core/util.h"

#define SET_NAME(class_name) setName(fmt::format(#class_name " {}", GetCount<class_name>()));

#endif  // DEBUG_MODE
