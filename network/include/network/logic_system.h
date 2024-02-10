#pragma once

#include "common/singleton.h"

namespace network {

class LogicSystem : public SingletonTemplate<LogicSystem> {
    friend class SingletonTemplate<LogicSystem>;
};

}  // namespace network
