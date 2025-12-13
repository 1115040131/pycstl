#pragma once

#include <glm/glm.hpp>

#include "monster_war/engine/scene/scene.h"

namespace pyc::monster_war {

class GameScene final : public Scene {
public:
    GameScene(Context& context);

    void init() override;
    void clean() override;

private:
    // --- 测试资源管理器 ---
    void testResourceManager();
};

}  // namespace pyc::monster_war