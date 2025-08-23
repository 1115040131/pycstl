#include "sunny_land/engine/scene/level_loader.h"

#include <filesystem>
#include <fstream>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/parallax_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/scene/scene.h"

namespace pyc::sunny_land {

bool LevelLoader::loadLevel(std::string_view level_path, Scene& scene) {
    map_path_ = level_path;

    // 1. 加载 JSON 文件
    std::ifstream file(level_path.data());
    if (!file.is_open()) {
        spdlog::error("无法打开关卡文件: {}", level_path);
        return false;
    }

    // 2. 解析 JSON 数据
    nlohmann::json json_data;
    try {
        file >> json_data;
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("解析 JSON 数据失败: {}", e.what());
        return false;
    }

    // 3. 加载图层数据
    if (!json_data.contains("layers") || !json_data["layers"].is_array()) {
        spdlog::error("地图文件 '{}' 中缺少或无效的 'layers' 数组。", level_path);
        return false;
    }
    for (const auto& layer_json : json_data["layers"]) {
        if (!layer_json.value("visible", true)) {
            spdlog::info("图层 '{}' 不可见，跳过加载。", layer_json.value("name", "Unnamed"));
            continue;
        }
        // 获取各图层对象中的类型（type）字段
        std::string layer_type = layer_json.value("type", "none");

        // 根据图层类型决定加载方法
        if (layer_type == "imagelayer") {
            loadImageLayer(layer_json, scene);
        } else if (layer_type == "tilelayer") {
            loadTileLayer(layer_json, scene);
        } else if (layer_type == "objectgroup") {
            loadObjectLayer(layer_json, scene);
        } else {
            spdlog::warn("不支持的图层类型: {}", layer_type);
        }
    }

    spdlog::info("关卡加载完成: {}", level_path);
    return true;
}

void LevelLoader::loadImageLayer(const nlohmann::json& layer_json, Scene& scene) {
    // 获取纹理相对路径 （会自动处理'\/'符号）
    std::string image_path = layer_json.value("image", "");
    if (image_path.empty()) {
        spdlog::error("图层 '{}' 缺少 'image' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }
    auto texture_id = resolvePath(image_path);

    // 获取图层偏移量，视差因子及重复标志
    auto offset = glm::vec2(layer_json.value("offsetx", 0.0f), layer_json.value("offsety", 0.0f));
    auto scroll_factor = glm::vec2(layer_json.value("parallaxx", 1.0f), layer_json.value("parallaxy", 1.0f));
    auto repeat = glm::vec2(layer_json.value("repeatx", false), layer_json.value("repeaty", false));

    // 获取图层名称
    auto layer_name = layer_json.value("name", "Unnamed");

    // 创建游戏对象
    auto game_object = std::make_unique<GameObject>(layer_name);
    game_object->addComponent<TransformComponent>(offset);
    game_object->addComponent<ParallaxComponent>(texture_id, scroll_factor, repeat);

    // 添加到场景中
    scene.addGameObject(std::move(game_object));
    spdlog::info("加载图层: '{}' 完成", layer_name);
}

void LevelLoader::loadTileLayer(const nlohmann::json&, Scene&) {}

void LevelLoader::loadObjectLayer(const nlohmann::json&, Scene&) {}

std::string LevelLoader::resolvePath(std::string_view image_path) {
    try {
        // 获取地图文件的父目录（相对于可执行文件）
        auto map_dir = std::filesystem::path(map_path_).parent_path();
        // 合并路径（相对于可执行文件）并返回。
        return std::filesystem::canonical(map_dir / image_path);
    } catch (const std::exception& e) {
        spdlog::error("解析路径失败: {}", e.what());
        return std::string(image_path);
    }
}

}  // namespace pyc::sunny_land