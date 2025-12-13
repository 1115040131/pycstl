#pragma once

#include <optional>
#include <vector>

#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>

#include "monster_war/engine/component/animation_component.h"
#include "monster_war/engine/component/sprite_component.h"

namespace pyc::monster_war {

/**
 * @brief 定义瓦片的类型，用于游戏逻辑（例如碰撞）。
 * @note 当前项目并未用到此信息
 */
enum class TileType {
    EMPTY,   ///< @brief 空白瓦片
    NORMAL,  ///< @brief 普通瓦片
    SOLID,   ///< @brief 静止可碰撞瓦片
    HAZARD,  ///< @brief 危险瓦片（例如火焰、尖刺等）
    // 未来补充其它类型
};

/**
 * @brief 瓦片信息，包含精灵、类型、动画和属性。
 * @note 它只是辅助LevelLoader解析的临时数据，不会保存在游戏中。
 */
struct TileInfo {
    Sprite sprite_;                             ///< @brief 精灵
    TileType type_;                             ///< @brief 类型
    std::optional<Animation> animation_;        ///< @brief 动画（支持Tiled动画图块）
    std::optional<nlohmann::json> properties_;  ///< @brief 属性（存放自定义属性，方便LevelLoader解析）
};

/**
 * @brief 瓦片层组件，包含瓦片大小、地图大小和瓦片实体列表。
 * @note 现在瓦片层更像一个容器，只是存储所有的“瓦片”，而每个瓦片就是一个实体。
 */
struct TileLayerComponent {
    glm::ivec2 tile_size_;             ///< @brief 瓦片大小
    glm::ivec2 map_size_;              ///< @brief 地图大小
    std::vector<entt::entity> tiles_;  ///< @brief 瓦片实体列表，每个瓦片对应一个实体，按顺序排列
};

}  // namespace pyc::monster_war