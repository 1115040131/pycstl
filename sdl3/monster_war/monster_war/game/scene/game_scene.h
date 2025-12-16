#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

#include "monster_war/engine/scene/scene.h"
#include "monster_war/engine/system/fwd.h"
#include "monster_war/game/data/waypoint_node.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/system/fwd.h"

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
    [[nodiscard]] bool initEventConnections();

    // 事件回调函数
    void onEnemyArriveHome(const EnemyArriveHomeEvent& event);

    // 测试函数
    void createTestEnemy();

private:
    std::unique_ptr<RenderSystem> render_system_;
    std::unique_ptr<MovementSystem> movement_system_;
    std::unique_ptr<AnimationSystem> animation_system_;
    std::unique_ptr<YSortSystem> ysort_system_;

    std::unique_ptr<FollowPathSystem> follow_path_system_;
    std::unique_ptr<RemoveDeadSystem> remove_dead_system_;

    std::unordered_map<int, WaypointNode> waypoint_nodes_;  // 路径节点ID到节点数据的映射
    std::vector<int> start_points_;                         // 起点ID列表
};

}  // namespace pyc::monster_war