#include "monster_war/engine/core/context.h"

#include <spdlog/spdlog.h>

namespace pyc::monster_war {
// clang-format off
Context::Context(entt::dispatcher& dispatcher,
                 ResourceManager& resource_manager,
                 Renderer& renderer,
                 Camera& camera,
                 TextRenderer& text_renderer,
                 InputManager& input_manager,
                 AudioPlayer& audio_player,
                 GameState& game_state,
                 Time& time)
    // clang-format on
    : dispatcher_(dispatcher),
      resource_manager_(resource_manager),
      renderer_(renderer),
      camera_(camera),
      text_renderer_(text_renderer),
      input_manager_(input_manager),
      audio_player_(audio_player),
      game_state_(game_state),
      time_(time) {
    spdlog::trace("上下文已创建并初始化。");
}

};  // namespace pyc::monster_war
