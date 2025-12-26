#include "monster_war/game/system/followpath_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <glm/geometric.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/component/velocity_component.h"
#include "monster_war/engine/utils/math.h"
#include "monster_war/game/component/blocked_by_component.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

void FollowPathSystem::update(entt::registry& registry, entt::dispatcher& dispatcher,
                              std::unordered_map<int, WaypointNode>& waypoint_nodes) {
    spdlog::trace("FollowPathSystem::update");
    // 筛选依据：速度组件、变换组件、敌人组件，排除“被阻挡的敌人”
    auto view =
        registry.view<VelocityComponent, TransformComponent, EnemyComponent>(entt::exclude<BlockedByComponent>);
    for (auto [entity, velocity, transform, enemy] : view.each()) {
        // 获取目标节点
        auto target_node = waypoint_nodes.at(enemy.target_waypoint_id_);

        // 计算当前位置到目标位置的向量
        auto direction = target_node.position_ - transform.position_;

        // 如果距离小于阈值，则切换到下一个节点（阈值不要太小，不然敌人速度快的话可能造成震荡）
        if (glm::length(direction) < 5.0f) {
            // 如果下一个节点ID列表为空，代表到达终点。则发送信号并添加删除标记
            auto size = target_node.next_node_ids_.size();
            if (size == 0) {
                spdlog::info("到达终点");
                // 发送信号并添加删除标记
                dispatcher.enqueue<EnemyArriveHomeEvent>();  // 具体做什么，由回调函数决定
                registry.emplace<DeadTag>(entity);           // 用于延迟删除
                continue;
            }
            // 随机选择下一个节点
            auto target_index = randomInt(0, size - 1);
            enemy.target_waypoint_id_ = target_node.next_node_ids_[target_index];
            // 更新目标节点与方向矢量
            target_node = waypoint_nodes.at(enemy.target_waypoint_id_);
            direction = target_node.position_ - transform.position_;
        }

        // 更新速度组件：velocity = 方向矢量 * speed
        velocity.velocity_ = glm::normalize(direction) * enemy.speed_;
    }
}

}  // namespace pyc::monster_war