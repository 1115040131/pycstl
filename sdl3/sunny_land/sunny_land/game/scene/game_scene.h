#pragma once

#include <fmt/format.h>
#include <glm/glm.hpp>

#include "sunny_land/engine/scene/scene.h"
#include "sunny_land/engine/utils/macro.h"

namespace pyc::sunny_land {

class SessionData;

/**
 * @brief 主要的游戏场景，包含玩家、敌人、关卡元素等。
 */
class GameScene final : public Scene {
public:
    GameScene(Context& context, SceneManager& scene_manager, std::shared_ptr<SessionData> session_data = nullptr);

    // 覆盖场景基类的核心方法
    void init() override;
    void handleInput() override;
    void update(std::chrono::duration<float> delta_time) override;
    void render() override;
    void clean() override;

private:
    [[nodiscard]] bool initLevel();         ///< @brief 初始化关卡
    [[nodiscard]] bool initPlayer();        ///< @brief 初始化玩家
    [[nodiscard]] bool initEnemyAndItem();  ///< @brief 初始化敌人和道具

    void handleObjectCollisions();        ///< @brief 处理游戏对象间的碰撞逻辑（从PhysicsEngine获取信息）
    void handleTileTriggers();            ///< @brief 处理瓦片触发事件（从PhysicsEngine获取信息）
    void handlePlayerDamage(int damage);  ///< @brief 处理玩家受伤（更新得分、UI等）
    void playerVSEnemyCollision(GameObject* player, GameObject* enemy);  ///< @brief 玩家与敌人碰撞处理
    void playerVSItemCollision(GameObject* player, GameObject* item);    ///< @brief 玩家与道具碰撞处理

    void toNextLevel(GameObject* trigger);  ///< @brief 进入下一个关卡

    /// @brief 根据关卡名称获取对应的地图文件路径
    std::string levelNameToPath(std::string_view level_name) const {
        return fmt::format(ASSET_PATH "maps/{}.tmj", level_name);
    }

    /**
     * @brief 创建一个特效对象（一次性）。
     * @param center_pos 特效中心位置
     * @param tag 特效标签（决定特效类型,例如"enemy","item"）
     */
    void createEffect(glm::vec2 center_pos, std::string_view tag);

    // for test
    void testSaveAndLoad();

private:
    std::shared_ptr<SessionData> game_session_data_;  ///< @brief 场景间共享数据，因此用shared_ptr
    GameObject* player_{};                            ///< @brief 保存玩家对象的指针，方便访问
};

}  // namespace pyc::sunny_land