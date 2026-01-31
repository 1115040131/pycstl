#include "monster_war/game/spawner/enemy_spawner.h"

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/utils/math.h"
#include "monster_war/game/data/level_config.h"
#include "monster_war/game/data/waypoint_node.h"
#include "monster_war/game/factory/entity_factory.h"

namespace pyc::monster_war {

EnemySpawner::EnemySpawner(entt::registry& registry, EntityFactory& entity_factory)
    : registry_(registry), entity_factory_(entity_factory) {}

EnemySpawner::~EnemySpawner() {}

void EnemySpawner::update(std::chrono::duration<float> delta_time) {
    auto& waves = registry_.ctx().get<Waves&>();

    // 如果“关卡波次队列”不为空，则考虑添加敌人
    if (!waves.waves_.empty()) {
        waves.next_wave_count_down_ -= delta_time;
        // 如果已经到了新的一波，则弹出并载入敌人波次队列
        if (waves.next_wave_count_down_ <= std::chrono::duration<float>::zero()) {
            auto& wave = waves.waves_.front();
            // 更新下一波次倒数计时器
            waves.next_wave_count_down_ = wave.next_wave_interval_;
            // 更新本波次敌人生成间隔与生成计时器
            spawn_interval_ = wave.spawn_interval_;
            spawn_timer_ = {};
            // 先把所有敌人依次加入“当前波次队列”
            for (auto [class_id, count] : wave.enemy_types_) {
                for (int i = 0; i < count; ++i) {
                    enemy_types_.push_back(class_id);
                }
            }
            // 打乱队列，确保敌人生成顺序随机
            shuffle(enemy_types_);

            // 本波次数据处理完毕，弹出关卡波次队列头
            waves.waves_.pop();
            spdlog::info("开始新一波敌人生成");
        }
    }

    // 如果“当前波次队列”不为空，则按“敌人生成间隔”生成敌人
    if (!enemy_types_.empty()) {
        spawn_timer_ += delta_time;
        if (spawn_timer_ >= spawn_interval_) {
            spawn_timer_ = {};
            spawnEnemy();  // 生成一个敌人
        }
    }
}

void EnemySpawner::spawnEnemy() {
    // 获取上下文数据
    auto& start_points = registry_.ctx().get<std::vector<int>&>();
    auto& waypoint_nodes = registry_.ctx().get<std::unordered_map<int, WaypointNode>&>();
    auto& level_config = registry_.ctx().get<std::shared_ptr<LevelConfig>&>();
    auto& level_number = registry_.ctx().get<int&>();

    // 随机选择起点
    auto random_index = randomInt(0, start_points.size() - 1);
    auto start_index = start_points[random_index];
    auto position = waypoint_nodes[start_index].position_;
    auto level = level_config->getEnemyLevel(level_number);
    auto rarity = level_config->getEnemyRarity(level_number);

    // 弹出敌人类型
    auto enemy_type = enemy_types_.front();
    enemy_types_.pop_front();

    // 创建敌人
    entity_factory_.createEnemyUnit(enemy_type, position, start_index, level, rarity);
    spdlog::info("创建敌人: 位置: {}, {}", position.x, position.y);
}

}  // namespace pyc::monster_war