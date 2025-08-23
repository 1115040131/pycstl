#pragma once

#include "sunny_land/engine/scene/scene.h"

namespace pyc::sunny_land {

/**
 * @brief 主要的游戏场景，包含玩家、敌人、关卡元素等。
 */
class GameScene final : public Scene {
public:
    GameScene(std::string_view name, Context& context, SceneManager& scene_manager);

    // 覆盖场景基类的核心方法
    void init() override;
    void handleInput() override;
    void update(std::chrono::duration<float> delta_time) override;
    void render() override;
    void clean() override;

private:
    // 测试函数
    // TODO: remove
    void creatTestObject();
    void testCamera();
    void testObject();
    GameObject* test_object_;
};

}  // namespace pyc::sunny_land