#include "monster_war/game/loader/entity_builder_mw.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/tilelayer_component.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

EntityBuilderMW::EntityBuilderMW(LevelLoader& level_loader, Context& context, entt::registry& registry,
                                 std::unordered_map<int, WaypointNode>& waypoint_nodes,
                                 std::vector<int>& start_points)
    : BasicEntityBuilder(level_loader, context, registry),
      waypoint_nodes_(waypoint_nodes),
      start_points_(start_points) {}

EntityBuilderMW* EntityBuilderMW ::build() {
    if (object_json_ && !tile_info_) {  // 代表自己绘制的形状,当前游戏只用到了路径节点
        buildPath();
    } else {
        BasicEntityBuilder::build();
        buildPlace();
    }

    return this;
}

void EntityBuilderMW::buildPath() {
    // 检查数据有效性
    if (!object_json_->value("point", false) || !object_json_->contains("properties") ||
        !object_json_->at("properties").is_array()) {
        return;
    }
    auto id = object_json_->value("id", 0);
    if (id == 0) {
        return;
    }

    // 解析数据并添加到容器
    auto position = glm::vec2(object_json_->value("x", 0.0f), object_json_->value("y", 0.0f));
    std::vector<int> next_node_ids;
    for (const auto& property : object_json_->at("properties")) {
        // 如果是对象类型, 且名称以 next 开头, 则添加到 next_node_ids
        if (property.value("type", "") == "object" && property.value("name", "").starts_with("next")) {
            auto next_node_id = property.value("value", 0);
            if (next_node_id != 0) {
                next_node_ids.push_back(next_node_id);
            }
        }
        // 如果名称是 start，且值为真，则将自身id添加到 start_points_ 中
        if (property.value("name", "") == "start" && property.value("value", false) == true) {
            start_points_.push_back(id);
        }
    }
    // 添加到节点容器中
    waypoint_nodes_[id] = WaypointNode{id, std::move(position), std::move(next_node_ids)};
    spdlog::trace("waypoint_nodes_ size: {}", waypoint_nodes_.size());
}

void EntityBuilderMW::buildPlace() {
    if (tile_info_ && tile_info_->properties_) {
        for (const auto& property : tile_info_->properties_.value()) {
            if (property.value("name", "") == "place") {
                auto type = property.value("value", "");
                if (type == "melee") {
                    registry_.emplace<MeleePlaceTag>(entity_id_);
                } else if (type == "range") {
                    registry_.emplace<RangedPlaceTag>(entity_id_);
                }
            }
        }
    }
}

}  // namespace pyc::monster_war