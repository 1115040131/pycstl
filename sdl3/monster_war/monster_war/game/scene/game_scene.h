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
    // --- 测试输入回调事件 (场景切换测试) ---
    int scene_num_{0};
    void onReplace();
    void onPush();
    void onPop();
    void onQuit();
};

}  // namespace pyc::monster_war