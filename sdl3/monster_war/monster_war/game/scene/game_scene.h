#pragma once

#include <glm/glm.hpp>

#include "monster_war/engine/scene/scene.h"

namespace pyc::monster_war {

class GameScene final : public Scene {
public:
    GameScene(Context& context, SceneManager& scene_manager);
};

}  // namespace pyc::monster_war