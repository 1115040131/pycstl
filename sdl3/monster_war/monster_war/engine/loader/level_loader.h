#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>

#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>

#include "common/noncopyable.h"
#include "monster_war/engine/loader/basic_entity_builder.h"
#include "monster_war/engine/utils/math.h"

namespace pyc::monster_war {

enum class TileType;
struct TileInfo;
class Scene;

class LevelLoader final : Noncopyable {
public:
    /// @brief 设置实体生成器（如果不设置，则使用默认的BasicEntityBuilder）
    void setEntityBuilder(std::unique_ptr<BasicEntityBuilder> builder) { entity_builder_ = std::move(builder); }

    /**
     * @brief 加载关卡数据，并生成游戏实体
     * @param level_path 关卡文件路径（.tmj）
     * @param scene 场景指针（非拥有）
     * @return true 加载成功，false 加载失败
     */
    [[nodiscard]] bool loadLevel(std::string_view level_path, Scene* scene);

    // --- getters and setters ---
    const glm::ivec2& getMapSize() const { return map_size_; }
    const glm::ivec2& getTileSize() const { return tile_size_; }
    int getCurrentLayer() const { return current_layer_; }

private:
    void loadImageLayer(const nlohmann::json& layer_json);   ///< @brief 加载图片图层
    void loadTileLayer(const nlohmann::json& layer_json);    ///< @brief 加载瓦片图层
    void loadObjectLayer(const nlohmann::json& layer_json);  ///< @brief 加载对象图层

    /**
     * @brief 加载 Tiled tileset 文件 (.tsj)，数据保存到tileset_data_。
     * @param tileset_path Tileset 文件路径。
     * @param first_gid 此 tileset 的第一个全局 ID。
     */
    void loadTileset(std::string_view tileset_path, int first_gid);

    /**
     * @brief 获取瓦片属性
     * @tparam T 属性类型
     * @param tile_json 瓦片json数据
     * @param property_name 属性名称
     * @return 属性值，如果属性不存在则返回 std::nullopt
     */
    template <typename T>
    std::optional<T> getTileProperty(const nlohmann::json& tile_json, std::string_view property_name) {
        if (!tile_json.contains("properties")) {
            return std::nullopt;
        }
        const auto& properties = tile_json["properties"];
        for (const auto& property : properties) {
            if (property.contains("name") && property["name"] == property_name) {
                if (property.contains("value")) {
                    return property["value"].get<T>();
                }
            }
        }
        return std::nullopt;
    }

    /**
     * @brief 获取瓦片碰撞器矩形
     * @param tile_json 瓦片json数据
     * @return 碰撞器矩形，如果碰撞器不存在则返回 std::nullopt
     */
    std::optional<Rect> getColliderRect(const nlohmann::json& tile_json) const;

    /**
     * @brief 获取瓦片纹理矩形（只针对单一图片图块集）
     * @param tileset_json 图块集json数据
     * @param local_id 图块集中的id
     * @return 纹理矩形
     */
    Rect getTextureRect(const nlohmann::json& tileset_json, int local_id) const;

    /**
     * @brief 根据瓦片json对象获取瓦片类型
     * @param tile_json 瓦片json数据
     * @return 瓦片类型
     */
    TileType getTileType(const nlohmann::json& tile_json) const;

    /**
     * @brief 根据图块集中的id获取瓦片类型（当前项目中，TileType无任何作用）
     * @param tileset_json 图块集json数据
     * @param local_id 图块集中的id
     * @return 瓦片类型
     */
    TileType getTileTypeById(const nlohmann::json& tileset_json, int local_id) const;

    /**
     * @brief 根据全局 ID 获取瓦片信息。
     * @param gid 全局 ID。
     * @return engine::component::TileInfo 瓦片信息。
     */
    std::optional<TileInfo> getTileInfoByGid(int gid) const;

    /**
     * @brief 解析图片路径，合并地图路径和相对路径。例如：
     * 1. 文件路径："assets/maps/level1.tmj"
     * 2. 相对路径："../textures/Layers/back.png"
     * 3. 最终路径："assets/textures/Layers/back.png"
     * @param relative_path 相对路径（相对于文件）
     * @param file_path 文件路径
     * @return std::string 解析后的完整路径。
     */
    std::string resolvePath(std::string_view relative_path, std::string_view file_path) const;

private:
    Scene* scene_;  ///< @brief 场景指针(非拥有)

    std::string map_path_;  ///< @brief 地图路径（拼接路径时需要）
    glm::ivec2 map_size_;   ///< @brief 地图尺寸(瓦片数量)
    glm::ivec2 tile_size_;  ///< @brief 瓦片尺寸(像素)

    std::map<int, nlohmann::json> tileset_data_;  ///< @brief firstgid -> 瓦片集数据

    std::unique_ptr<BasicEntityBuilder> entity_builder_;  ///< @brief 实体生成器(生成器模式)

    int current_layer_ = 0;  ///< @brief 当前图层序号（用于RenderComponent，决定渲染顺序）
};

}  // namespace pyc::monster_war
