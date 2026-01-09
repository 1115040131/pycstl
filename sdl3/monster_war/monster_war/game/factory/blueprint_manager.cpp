#include "monster_war/game/factory/blueprint_manager.h"

#include <filesystem>
#include <fstream>

#include <entt/core/hashed_string.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/resource/resource_manager.h"

namespace pyc::monster_war {

BlueprintManager::BlueprintManager(ResourceManager& resource_manager) : resource_manager_(resource_manager) {}

bool BlueprintManager::loadPlayerClassBlueprints(std::string_view player_json_path) {
    auto path = std::filesystem::path(player_json_path);
    std::ifstream file(path);
    nlohmann::json json;
    file >> json;
    file.close();
    // --- 解析蓝图 ---
    try {
        for (const auto& [class_name, data_json] : json.items()) {
            entt::id_type class_id = entt::hashed_string(class_name.c_str());
            player_class_blueprints_.emplace(class_id, PlayerClassBlueprint{
                                                           class_id,
                                                           parseProjectileID(data_json),
                                                           class_name,
                                                           parseStats(data_json),
                                                           parsePlayer(data_json),
                                                           parseSound(data_json),
                                                           parseSprite(data_json),
                                                           parseDisplayInfo(data_json),
                                                           parseAnimationsMap(data_json),
                                                       });
        }
    } catch (const std::exception& e) {
        spdlog::error("加载玩家单位数据时出错: {}", e.what());
        return false;
    }
    return true;
}

bool BlueprintManager::loadEnemyClassBlueprints(std::string_view enemy_json_path) {
    auto path = std::filesystem::path(enemy_json_path);
    std::ifstream file(path);
    nlohmann::json json;
    file >> json;
    file.close();
    // --- 解析蓝图 ---
    try {
        for (const auto& [class_name, data_json] : json.items()) {
            entt::id_type class_id = entt::hashed_string(class_name.c_str());
            enemy_class_blueprints_.emplace(class_id, EnemyClassBlueprint{
                                                          class_id,
                                                          parseProjectileID(data_json),
                                                          class_name,
                                                          parseStats(data_json),
                                                          parseEnemy(data_json),
                                                          parseSound(data_json),
                                                          parseSprite(data_json),
                                                          parseDisplayInfo(data_json),
                                                          parseAnimationsMap(data_json),
                                                      });
        }
    } catch (const std::exception& e) {
        spdlog::error("加载敌人单位数据时出错: {}", e.what());
        return false;
    }
    return true;
}

bool BlueprintManager::loadProjectileBlueprints(std::string_view projectile_json_path) {
    auto path = std::filesystem::path(projectile_json_path);
    std::ifstream file(path);
    nlohmann::json json;
    file >> json;
    file.close();
    // --- 解析蓝图 ---
    try {
        for (const auto& [name, data_json] : json.items()) {
            // 解析基础数据
            entt::id_type id = entt::hashed_string(name.c_str());
            // 解析其它数据，组合蓝图并插入容器
            projectile_blueprints_.emplace(
                id, ProjectileBlueprint{
                        id,
                        name,
                        data_json["arc_height"].get<float>(),
                        std::chrono::duration<float>(data_json["total_flight_time"].get<float>()),
                        parseSprite(data_json),
                        parseSound(data_json),
                    });
        }
    } catch (const std::exception& e) {
        spdlog::error("加载投射物数据时出错: {}", e.what());
        return false;
    }
    return true;
}

const PlayerClassBlueprint& BlueprintManager::getPlayerClassBlueprint(entt::id_type id) const {
    if (auto it = player_class_blueprints_.find(id); it != player_class_blueprints_.end()) {
        return it->second;
    }
    spdlog::error("未找到对应 id 的 PlayerClassBlueprint: {}", id);
    return player_class_blueprints_.begin()->second;
}

const EnemyClassBlueprint& BlueprintManager::getEnemyClassBlueprint(entt::id_type id) const {
    if (auto it = enemy_class_blueprints_.find(id); it != enemy_class_blueprints_.end()) {
        return it->second;
    }
    spdlog::error("未找到对应 id 的 EnemyClassBlueprint: {}", id);
    return enemy_class_blueprints_.begin()->second;
}

const ProjectileBlueprint& BlueprintManager::getProjectileBlueprint(entt::id_type id) const {
    if (auto it = projectile_blueprints_.find(id); it != projectile_blueprints_.end()) {
        return it->second;
    }
    spdlog::error("未找到对应 id 的 ProjectileBlueprint: {}", id);
    return projectile_blueprints_.begin()->second;
}

entt::id_type BlueprintManager::parseProjectileID(const nlohmann::json& json) {
    if (json.contains("projectile")) {
        return entt::hashed_string(json["projectile"].get<std::string>().c_str());
    }
    return entt::null;
}

StatsBlueprint BlueprintManager::parseStats(const nlohmann::json& json) {
    return {
        json["hp"].get<float>(),
        json["atk"].get<float>(),
        json["def"].get<float>(),
        json["range"].get<float>(),
        std::chrono::duration<float>(json["atk_interval"].get<float>()),
    };
}

SpriteBlueprint BlueprintManager::parseSprite(const nlohmann::json& json) {
    auto width = json["width"].get<float>();
    auto height = json["height"].get<float>();
    auto sprite_path = json["sprite_sheet"].get<std::string>();
    // 可选部分：源矩形的起点默认值为 0,0，渲染目标大小默认值为 width,height
    // （如果指定，起点为 x,y，渲染目标大小为 size_x,size_y）
    return {
        entt::hashed_string(sprite_path.c_str()),
        sprite_path,
        {
            glm::vec2(json.value("x", 0), json.value("y", 0)),
            glm::vec2(width, height),
        },
        glm::vec2(json.value("size_x", width), json.value("size_y", height)),
        glm::vec2(json.value("offset_x", 0), json.value("offset_y", 0)),
        json.value("face_right", true),
    };
}

std::unordered_map<entt::id_type, AnimationBlueprint> BlueprintManager::parseAnimationsMap(
    const nlohmann::json& json) {
    std::unordered_map<entt::id_type, AnimationBlueprint> animations;  // 先准备好容器
    for (const auto& [anim_name, anim_data] : json["animation"].items()) {
        // 处理可能存在的事件信息
        std::unordered_map<size_t, entt::id_type> events;
        if (anim_data.contains("events")) {
            for (const auto& [event_name, event_frame] : anim_data["events"].items()) {
                events.emplace(event_frame.get<size_t>(), entt::hashed_string(event_name.c_str()));
            }
        }
        animations.emplace(entt::hashed_string(anim_name.c_str()),
                           AnimationBlueprint{
                               std::chrono::duration<double, std::milli>(anim_data.value("duration", 100.0f)),
                               anim_data.value("row", 0),
                               anim_data["frames"].get<std::vector<size_t>>(),
                               std::move(events),
                           });
    }
    return animations;
}

SoundBlueprint BlueprintManager::parseSound(const nlohmann::json& json) {
    SoundBlueprint sounds{};
    if (json.contains("sounds")) {
        for (const auto& [sound_name, sound_data] : json["sounds"].items()) {
            // 先把 sound_value 看成是音效路径并通过资源管理器加载
            auto sound_path = sound_data.get<std::string>();
            auto sound_id = entt::hashed_string(sound_path.c_str());
            resource_manager_.loadSound(sound_id, sound_path);
            // 将音效键值对转换为音效ID并插入到声音蓝图中
            sounds.sounds_.emplace(entt::hashed_string(sound_name.c_str()), sound_id);
        }
    }
    return sounds;
}

PlayerBlueprint BlueprintManager::parsePlayer(const nlohmann::json& json) {
    // 解析类型
    auto type_str = json["type"].get<std::string>();
    auto type = type_str == "melee" ? PlayerType::MELEE :  // 三目运算符嵌套
                    type_str == "ranged" ? PlayerType::RANGED
                : type_str == "mixed"    ? PlayerType::MIXED
                                         : PlayerType::UNKNOWN;
    spdlog::info("type_str: {}, type: {}", type_str, static_cast<int>(type));
    // 解析技能
    entt::id_type skill_id = entt::null;
    if (json.contains("skill")) {
        skill_id = entt::hashed_string(json["skill"].get<std::string>().c_str());
    }
    // 解析其他数据并返回
    return {
        type, skill_id, json["healer"].get<bool>(), json["block"].get<int>(), json["cost"].get<int>(),
    };
}

EnemyBlueprint BlueprintManager::parseEnemy(const nlohmann::json& json) {
    // 敌人组件蓝图只包含“是否远程”和“移动速度”
    return {
        json["ranged"].get<bool>(),
        json["speed"].get<float>(),
    };
}

DisplayInfoBlueprint BlueprintManager::parseDisplayInfo(const nlohmann::json& json) {
    // 显示信息蓝图只包含“名称”和“描述”
    return {
        json.value("name", ""),
        json.value("description", ""),
    };
}

}  // namespace pyc::monster_war