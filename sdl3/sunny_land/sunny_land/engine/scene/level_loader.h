#pragma once

#include <map>
#include <string>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

class Scene;
class TileInfo;

class LevelLoader final : Noncopyable {
public:
    /**
     * @brief 加载关卡数据到指定的 Scene 对象中。
     * @param level_path Tiled JSON 地图文件的路径。
     * @param scene 要加载数据的目标 Scene 对象。
     * @return bool 是否加载成功。
     */
    [[nodiscard]] bool loadLevel(std::string_view level_path, Scene& scene);

private:
    void loadImageLayer(const nlohmann::json& layer_json, Scene& scene);   ///< @brief 加载图片图层
    void loadTileLayer(const nlohmann::json& layer_json, Scene& scene);    ///< @brief 加载瓦片图层
    void loadObjectLayer(const nlohmann::json& layer_json, Scene& scene);  ///< @brief 加载对象图层

    /**
     * @brief 根据全局 ID 获取瓦片信息。
     * @param gid 全局 ID。
     * @return engine::component::TileInfo 瓦片信息。
     */
    TileInfo getTileInfoByGid(int gid);

    /**
     * @brief 加载 Tiled tileset 文件 (.tsj)。
     * @param tileset_path Tileset 文件路径。
     * @param first_gid 此 tileset 的第一个全局 ID。
     */
    void loadTileset(std::string_view tileset_path, int first_gid);

    /**
     * @brief 解析图片路径，合并地图路径和相对路径。例如：
     * 1. 文件路径："assets/maps/level1.tmj"
     * 2. 相对路径："../textures/Layers/back.png"
     * 3. 最终路径："assets/textures/Layers/back.png"
     * @param relative_path 相对路径（相对于文件）
     * @param file_path 文件路径
     * @return std::string 解析后的完整路径。
     */
    std::string resolvePath(std::string_view relative_path, std::string_view file_path);

private:
    std::string map_path_;                        ///< @brief 地图路径（拼接路径时需要）
    glm::ivec2 map_size_;                         ///< @brief 地图尺寸(瓦片数量)
    glm::ivec2 tile_size_;                        ///< @brief 瓦片尺寸(像素)
    std::map<int, nlohmann::json> tileset_data_;  ///< @brief firstgid -> 瓦片集数据
};

}  // namespace pyc::sunny_land
