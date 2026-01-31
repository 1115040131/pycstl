#pragma once

#include <chrono>
#include <deque>

#include <entt/entity/fwd.hpp>

namespace pyc::monster_war {

class EntityFactory;

/**
 * @brief 敌人生成器，根据波次数据生成敌人
 */
class EnemySpawner {
public:
    /**
     * @brief 构造函数
     * @param registry entt注册表
     * @param entity_factory 实体工厂
     */
    EnemySpawner(entt::registry& registry, EntityFactory& entity_factory);
    ~EnemySpawner();

    void update(std::chrono::duration<float> delta_time);

private:
    void spawnEnemy();

private:
    entt::registry& registry_;
    EntityFactory& entity_factory_;

    std::chrono::duration<float> spawn_timer_{};     ///< @brief 波次内生成计时器
    std::chrono::duration<float> spawn_interval_{};  ///< @brief 波次内生成间隔
    std::deque<entt::id_type> enemy_types_;  ///< @brief 波次内敌人队列 (使用双端队列是为了支持随机打乱顺序)
};

}  // namespace pyc::monster_war