#include "sunny_land/engine/core/context.h"

#include <spdlog/spdlog.h>

namespace pyc::sunny_land {

Context::Context(ResourceManager& resource_manager, Renderer& renderer, Camera& camera,
                 InputManager& input_manager)
    : resource_manager_(resource_manager), renderer_(renderer), camera_(camera), input_manager_(input_manager) {
    spdlog::trace("上下文已创建并初始化。");
}

};  // namespace pyc::sunny_land
