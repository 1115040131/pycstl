#include "sunny_land/engine/component/tilelayer_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/render/renderer.h"

namespace pyc::sunny_land {

TileLayerComponent::TileLayerComponent(glm::ivec2 tile_size, glm::ivec2 map_size, std::vector<TileInfo>&& tiles)
    : tile_size_(std::move(tile_size)), map_size_(std::move(map_size)), tiles_(std::move(tiles)) {
    if (tile_size_.x <= 0 || tile_size_.y <= 0) {
        spdlog::error("TileLayerComponent: 无效的瓦片尺寸 ({}, {})。瓦片数据将被清除。", tile_size_.x,
                      tile_size_.y);
        tiles_.clear();
        map_size_ = {0, 0};
    }
    if (tiles_.size() != static_cast<size_t>(map_size_.x * map_size_.y)) {
        spdlog::error("TileLayerComponent: 地图尺寸与提供的瓦片向量大小不匹配。瓦片数据将被清除。");
        tiles_.clear();
        map_size_ = {0, 0};
    }
    spdlog::trace("TileLayerComponent 构造完成");
}

void TileLayerComponent::init() {
    if (!owner_) {
        spdlog::warn("TileLayerComponent 的 owner_ 未设置。");
    }
    spdlog::trace("TileLayerComponent 初始化完成");
}

void TileLayerComponent::render(Context& context) {
    for (int y = 0; y < map_size_.y; ++y) {
        for (int x = 0; x < map_size_.x; ++x) {
            auto index = static_cast<size_t>(y * map_size_.x + x);
            // 检查索引有效性以及瓦片是否需要渲染
            if (index < tiles_.size() && tiles_[index].type != TileType::EMPTY) {
                const auto& tile_info = tiles_[index];
                // 计算该瓦片在世界中的左上角位置 (drawSprite 预期接收左上角坐标)
                auto tile_pos = glm::vec2{
                    offset_.x + static_cast<float>(x) * tile_size_.x,
                    offset_.y + static_cast<float>(y) * tile_size_.y,
                };
                // 但如果图片的大小与瓦片的大小不一致，需要调整 y 坐标 (瓦片层的对齐点是左下角)
                if (static_cast<int>(tile_info.sprite.getSourceRect()->h) != tile_size_.y) {
                    tile_pos.y -= static_cast<float>(tile_info.sprite.getSourceRect()->h) - tile_size_.y;
                }
                // 执行绘制
                context.getRenderer().drawSprite(context.getCamera(), tile_info.sprite, tile_pos);
            }
        }
    }
}

const TileInfo* TileLayerComponent::getTileInfoAt(glm::ivec2 pos) const {
    if (pos.x < 0 || pos.x >= map_size_.x || pos.y < 0 || pos.y >= map_size_.y) {
        spdlog::warn("TileLayerComponent: 瓦片坐标越界: ({}, {})", pos.x, pos.y);
        return nullptr;
    }
    auto index = static_cast<size_t>(pos.y * map_size_.x + pos.x);
    if (index < tiles_.size()) {
        return &tiles_[index];
    }
    spdlog::warn("TileLayerComponent: 瓦片索引越界: {}", index);
    return nullptr;
}

TileType TileLayerComponent::getTileTypeAt(glm::ivec2 pos) const {
    const TileInfo* tile_info = getTileInfoAt(pos);
    return tile_info ? tile_info->type : TileType::EMPTY;
}

TileType TileLayerComponent::getTileTypeAtWorldPos(const glm::vec2& world_pos) const {
    glm::vec2 relative_pos = world_pos - offset_;

    auto tile_x = static_cast<int>(std::floor(relative_pos.x / tile_size_.x));
    auto tile_y = static_cast<int>(std::floor(relative_pos.y / tile_size_.y));
    return getTileTypeAt({tile_x, tile_y});
}

}  // namespace pyc::sunny_land