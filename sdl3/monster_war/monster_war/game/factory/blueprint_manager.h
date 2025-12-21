#pragma once

#include <entt/entity/fwd.hpp>
#include <nlohmann/json_fwd.hpp>

#include "monster_war/game/data/entity_blueprint.h"

namespace pyc::monster_war {

class ResourceManager;

/**
 * @brief 蓝图管理器，用于存储、管理所有蓝图
 *
 * 它从json数据中加载蓝图并保存到容器，并和获取蓝图的功能。蓝图信息将由实体工厂使用。
 */
class BlueprintManager {
    friend class EntityFactory;

public:
    explicit BlueprintManager(ResourceManager& resource_manager);

    ///< @brief 加载敌人类型蓝图, 返回是否成功
    [[nodiscard]] bool loadEnemyClassBlueprints(std::string_view enemy_json_path);

    ///< @brief 获取指定ID的敌人类型蓝图
    const EnemyClassBlueprint& getEnemyClassBlueprint(entt::id_type id) const;

private:
    // --- 分别针对各个子蓝图进行json解析，并创建(返回)对应的蓝图结构体 ---
    StatsBlueprint parseStats(const nlohmann::json& json);
    SpriteBlueprint parseSprite(const nlohmann::json& json);
    std::unordered_map<entt::id_type, AnimationBlueprint> parseAnimationsMap(const nlohmann::json& json);
    SoundBlueprint parseSound(const nlohmann::json& json);
    EnemyBlueprint parseEnemy(const nlohmann::json& json);
    DisplayInfoBlueprint parseDisplayInfo(const nlohmann::json& json);

private:
    ResourceManager& resource_manager_;

    std::unordered_map<entt::id_type, EnemyClassBlueprint> enemy_class_blueprints_;  ///< @brief 敌人类型蓝图
};

}  // namespace pyc::monster_war