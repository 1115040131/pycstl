#pragma once

#include "monster_war/engine/loader/basic_entity_builder.h"
#include "monster_war/game/data/waypoint_node.h"

namespace pyc::monster_war {

/**
 * @brief 拓展的关卡载入实体生成器
 * 功能包括：
 * 1. 生成路径节点和起点。
 * 2. ...
 */
class EntityBuilderMW : public BasicEntityBuilder {
public:
    /**
     * @brief 构造函数
     * @param level_loader 关卡载入器
     * @param context 上下文
     * @param registry 实体注册表
     * @param waypoint_nodes 路径节点
     * @param start_points 起点
     */
    EntityBuilderMW(LevelLoader& level_loader, Context& context, entt::registry& registry,
                    std::unordered_map<int, WaypointNode>& waypoint_nodes, std::vector<int>& start_points);
    ~EntityBuilderMW() = default;

    EntityBuilderMW* build() override;

private:
    void buildPath();   ///< @brief 生成路径节点
    void buildPlace();  ///< @brief 生成单位放置区域标签

private:
    // 保存路径节点和起点数据（非拥有）
    std::unordered_map<int, WaypointNode>& waypoint_nodes_;
    std::vector<int>& start_points_;
};

}  // namespace pyc::monster_war