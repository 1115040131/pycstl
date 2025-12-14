#pragma once

#include "monster_war/engine/scene/scene.h"
#include "monster_war/engine/system/fwd.h"

namespace pyc::monster_war {

class GameScene final : public Scene {
public:
    GameScene(Context& context);
    ~GameScene() override;

    void init() override;
    void update(std::chrono::duration<float> delta_time) override;
    void render() override;
    void clean() override;

private:
    [[nodiscard]] bool loadLevel();

private:
    std::unique_ptr<RenderSystem> render_system_;
    std::unique_ptr<MovementSystem> movement_system_;
    std::unique_ptr<AnimationSystem> animation_system_;
    std::unique_ptr<YSortSystem> ysort_system_;
};

}  // namespace pyc::monster_war