module;

#include <spdlog/spdlog.h>

module sunny_land.engine.core;

import sunny_land.engine.audio.audio_player;
import sunny_land.engine.core.game_state;
import sunny_land.engine.input.input_manager;
import sunny_land.engine.resource;

namespace pyc::sunny_land {

Context::Context(ResourceManager& resource_manager, Renderer& renderer, Camera& camera,
                 TextRenderer& text_renderer, InputManager& input_manager, PhysicsEngine& physics_engine,
                 AudioPlayer& audio_player, GameState& game_state)
    : resource_manager_(resource_manager),
      renderer_(renderer),
      camera_(camera),
      text_renderer_(text_renderer),
      input_manager_(input_manager),
      physics_engine_(physics_engine),
      audio_player_(audio_player),
      game_state_(game_state) {
    spdlog::trace("上下文已创建并初始化。");
}

}  // namespace pyc::sunny_land
