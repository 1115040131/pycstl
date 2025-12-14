#include "monster_war/engine/loader/level_loader.h"

#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>

#include "monster_war/engine/component/name_component.h"
#include "monster_war/engine/component/parallax_component.h"
#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/component/tilelayer_component.h"
#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/resource/resource_manager.h"
#include "monster_war/engine/scene/scene.h"

namespace pyc::monster_war {

bool LevelLoader::loadLevel(std::string_view level_path, Scene* scene) {
    if (!scene) {
        spdlog::error("场景指针为空");
        return false;
    }
    scene_ = scene;

    if (!entity_builder_) {
        spdlog::info("设置默认的实体生成器");
        entity_builder_ = std::make_unique<BasicEntityBuilder>(*this, scene->getContext(), scene->getRegistry());
    }

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

    // 3. 获取基本地图信息 (名称、地图尺寸、瓦片尺寸)
    map_path_ = level_path;
    map_size_ = glm::ivec2(json_data.value("width", 0), json_data.value("height", 0));
    tile_size_ = glm::ivec2(json_data.value("tilewidth", 0), json_data.value("tileheight", 0));

    // 4. 加载 tileset 数据
    if (json_data.contains("tilesets") && json_data["tilesets"].is_array()) {
        for (const auto& tileset_json : json_data["tilesets"]) {
            auto first_gid = tileset_json.value("firstgid", -1);
            auto source = tileset_json.value("source", "");
            if (first_gid < 0 || source.empty()) {
                spdlog::error("tilesets 对象中缺少有效 'source' 或 'firstgid' 字段。");
                continue;
            }
            loadTileset(resolvePath(source, map_path_), first_gid);
        }
    }

    // 5. 加载图层数据
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
            loadImageLayer(layer_json);
        } else if (layer_type == "tilelayer") {
            loadTileLayer(layer_json);
        } else if (layer_type == "objectgroup") {
            loadObjectLayer(layer_json);
        } else {
            spdlog::warn("不支持的图层类型: {}", layer_type);
        }
    }

    spdlog::info("关卡加载完成: {}", level_path);
    return true;
}

void LevelLoader::loadImageLayer(const nlohmann::json& layer_json) {
    // 获取纹理相对路径 （会自动处理'\/'符号）
    std::string image_path = layer_json.value("image", "");
    if (image_path.empty()) {
        spdlog::error("图层 '{}' 缺少 'image' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }

    // 创建精灵 (在获取纹理大小时会确保纹理加载)
    auto texture_path = resolvePath(image_path, map_path_);
    auto& resource_manager = scene_->getContext().getResourceManager();
    auto texture_size = resource_manager.getTextureSize(entt::hashed_string(texture_path.c_str()), texture_path);
    auto sprite = Sprite(texture_path, Rect{glm::vec2(0.0f), texture_size});

    // 获取图层偏移量，视差因子及重复标志
    auto offset = glm::vec2(layer_json.value("offsetx", 0.0f), layer_json.value("offsety", 0.0f));
    auto scroll_factor = glm::vec2(layer_json.value("parallaxx", 1.0f), layer_json.value("parallaxy", 1.0f));
    auto repeat = glm::vec2(layer_json.value("repeatx", false), layer_json.value("repeaty", false));

    // 获取图层名称
    const auto& layer_name = layer_json.value("name", "Unnamed");
    auto name_id = entt::hashed_string(layer_name.c_str());

    // 创建图层实体
    auto& registry = scene_->getRegistry();
    auto entity = registry.create();

    // 添加组件
    registry.emplace<NameComponent>(entity, name_id, layer_name);
    registry.emplace<TransformComponent>(entity, offset);
    registry.emplace<ParallaxComponent>(entity, scroll_factor, repeat);
    registry.emplace<SpriteComponent>(entity, std::move(sprite));
    /* 实体与组件创建完毕后即由registry自动管理，不需要“添加到场景”的步骤 */

    spdlog::info("加载图层: '{}' 完成", layer_name);
}

void LevelLoader::loadTileLayer(const nlohmann::json& layer_json) {
    if (!layer_json.contains("data") || !layer_json["data"].is_array()) {
        spdlog::error("图层 '{}' 缺少 'data' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }

    // 获取图层名称
    std::string layer_name = layer_json.value("name", "Unnamed");
    entt::id_type name_id = entt::hashed_string(layer_name.c_str());

    // 创建图层实体
    auto& registry = scene_->getRegistry();
    auto layer_entity = registry.create();
    registry.emplace<NameComponent>(layer_entity, name_id, layer_name);

    // 准备 TileInfo Vector (瓦片数量 = 地图宽度 * 地图高度)
    std::vector<entt::entity> tiles;
    tiles.reserve(map_size_.x * map_size_.y);

    size_t index = 0;  // data数据的索引，它决定图块在地图中的位置
    // --- 每一个瓦片都是一个独立的entity ---
    for (const int gid : layer_json["data"]) {
        if (gid == 0) {
            index++;
            continue;
        }
        auto tile_info_opt = getTileInfoByGid(gid);
        if (!tile_info_opt) {
            spdlog::error("瓦片 ID 为 {} 的瓦片未找到图块集。", gid);
            index++;
            continue;
        }
        // 使用生成器创建瓦片实体, 添加到vector中
        tiles.push_back(entity_builder_->configure(index, &tile_info_opt.value())->build()->getEntityID());
        index++;
    }

    // 最后将瓦片层组件添加到图层实体中
    registry.emplace<TileLayerComponent>(layer_entity, tile_size_, map_size_, std::move(tiles));

    spdlog::info("加载图层: '{}' 完成", layer_name);
}

void LevelLoader::loadObjectLayer(const nlohmann::json& layer_json) {
    if (!layer_json.contains("objects") || !layer_json["objects"].is_array()) {
        spdlog::error("图层 '{}' 缺少 'objects' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }
    for (const auto& object : layer_json["objects"]) {
        // 获取对象gid
        auto gid = object.value("gid", 0);
        if (gid == 0) {  // 如果gid为0 (即不存在)，则代表自己绘制的形状
            // 配置生成器，并调用build，针对自定义形状
            entity_builder_->configure(&object)->build();
        } else {  // 如果gid存在，则按照图片解析流程
            // 配置生成器，针对图片对象
            auto tile_info = getTileInfoByGid(gid);
            if (!tile_info) {
                spdlog::warn("对象图层 '{}' 中的对象缺少有效的 'gid' 或瓦片信息。",
                             layer_json.value("name", "Unnamed"));
                continue;
            }
            // 配置生成器，并调用build，针对图片对象
            entity_builder_->configure(&object, &tile_info.value())->build();
        }
    }
}

// void LevelLoader::addAnimation(const nlohmann::json& anim_json, AnimationComponent* animation_component,
//                                const glm::vec2& sprite_size) {
//     // 检查 anim_json 必须是一个对象，并且 ac 不能为 nullptr
//     if (!anim_json.is_object() || !animation_component) {
//         spdlog::error("无效的动画 JSON 或 AnimationComponent 指针。");
//         return;
//     }
//     // 遍历动画 JSON 对象中的每个键值对（动画名称 : 动画信息）
//     for (const auto& anim : anim_json.items()) {
//         std::string_view anim_name = anim.key();
//         const auto& anim_info = anim.value();
//         if (!anim_info.is_object()) {
//             spdlog::warn("动画 '{}' 的信息无效或为空。", anim_name);
//             continue;
//         }
//         // 获取可能存在的动画帧信息
//         auto duration_ms = std::chrono::milliseconds(anim_info.value("duration", 100));  //
//         默认持续时间为100毫秒 auto duration =
//         std::chrono::duration_cast<std::chrono::duration<float>>(duration_ms);  // 转换为秒 auto row =
//         anim_info.value("row", 0);                                                   // 默认行数为0
//         // 帧信息（数组）是必须存在的
//         if (!anim_info.contains("frames") || !anim_info["frames"].is_array()) {
//             spdlog::warn("动画 '{}' 缺少 'frames' 数组。", anim_name);
//             continue;
//         }
//         // 创建一个Animation对象 (默认为循环播放)
//         auto animation = std::make_unique<Animation>(anim_name);

//         // 遍历数组并进行添加帧信息到animation对象
//         for (const auto& frame : anim_info["frames"]) {
//             if (!frame.is_number_integer()) {
//                 spdlog::warn("动画 {} 中 frames 数组格式错误！", anim_name);
//                 continue;
//             }
//             auto column = frame.get<int>();
//             // 计算源矩形
//             SDL_FRect src_rect = {
//                 column * sprite_size.x,
//                 row * sprite_size.y,
//                 sprite_size.x,
//                 sprite_size.y,
//             };
//             // 添加动画帧到 Animation
//             animation->addFrame(src_rect, duration);
//         }
//         // 将 Animation 对象添加到 AnimationComponent 中
//         animation_component->addAnimation(std::move(animation));
//     }
// }

// void LevelLoader::addSound(const nlohmann::json& sound_json, AudioComponent* audio_component) {
//     if (!sound_json.is_object() || !audio_component) {
//         spdlog::error("无效的音效 JSON 或 AudioComponent 指针。");
//         return;
//     }
//     // 遍历音效 JSON 对象中的每个键值对（音效id : 音效路径）
//     for (const auto& sound : sound_json.items()) {
//         const std::string& sound_id = sound.key();
//         const std::string& sound_path = sound.value();
//         if (sound_id.empty() || sound_path.empty()) {
//             spdlog::warn("音效 '{}' 缺少必要信息。", sound_id);
//             continue;
//         }
//         // 添加音效到 AudioComponent
//         audio_component->addSound(sound_id, resolvePath(sound_path, map_path_));
//     }
// }

std::optional<Rect> LevelLoader::getColliderRect(const nlohmann::json& tile_json) const {
    if (!tile_json.contains("objectgroup")) {
        return std::nullopt;
    }
    const auto& objectgroup = tile_json["objectgroup"];
    if (!objectgroup.contains("objects")) {
        return std::nullopt;
    }
    const auto& objects = objectgroup["objects"];
    for (const auto& object : objects) {
        auto rect = Rect{
            {object.value("x", 0.0f), object.value("y", 0.0f)},
            {object.value("width", 0.0f), object.value("height", 0.0f)},
        };
        if (rect.size.x > 0 && rect.size.y > 0) {
            return rect;
        }
    }
    return std::nullopt;
}

Rect LevelLoader::getTextureRect(const nlohmann::json& tileset_json, int local_id) const {
    auto columns = tileset_json.value("columns", 1);
    auto tile_width = tileset_json.value("tilewidth", 0);
    auto tile_height = tileset_json.value("tileheight", 0);
    auto coordinate_x = local_id % columns;
    auto coordinate_y = local_id / columns;
    return Rect{glm::vec2(coordinate_x * tile_width, coordinate_y * tile_height),
                glm::vec2(tile_width, tile_height)};
}

TileType LevelLoader::getTileType(const nlohmann::json& tile_json) const {
    if (tile_json.contains("properties")) {
        for (const auto& property : tile_json["properties"]) {
            auto name = property.value("name", "");
            if (name == "solid") {
                return property.value("value", false) ? TileType::SOLID : TileType::NORMAL;
            } else if (name == "hazard") {
                return property.value("value", false) ? TileType::HAZARD : TileType::NORMAL;
            }
        }
    }
    return TileType::NORMAL;
}

TileType LevelLoader::getTileTypeById(const nlohmann::json& tileset_json, int local_id) const {
    if (tileset_json.contains("tiles")) {
        for (const auto& tile : tileset_json["tiles"]) {
            if (tile.contains("id") && tile["id"] == local_id) {
                return getTileType(tile);
            }
        }
    }
    return TileType::NORMAL;
}

std::optional<TileInfo> LevelLoader::getTileInfoByGid(int gid) const {
    if (gid == 0) {
        return std::nullopt;
    }

    // upper_bound：查找tileset_data_中键大于 gid 的第一个元素，返回迭代器
    auto tileset_it = tileset_data_.upper_bound(gid);
    if (tileset_it == tileset_data_.begin()) {
        spdlog::error("gid为 {} 的瓦片未找到图块集。", gid);
        return std::nullopt;
    }
    --tileset_it;  // 前移一个位置，这样就得到不大于gid的最近一个元素（我们需要的）

    const auto& tileset = tileset_it->second;
    auto local_id = gid - tileset_it->first;
    const auto& file_path = tileset.value("file_path", "");  // 获取图块集文件路径
    if (file_path.empty()) {
        spdlog::error("Tileset 文件 '{}' 缺少 'file_path' 属性。", tileset_it->first);
        return std::nullopt;
    }
    if (tileset.contains("image")) {  // 这是单一图片的情况
        // 获取必要信息
        auto texture_rect = getTextureRect(tileset, local_id);
        auto image_path = tileset["image"].get<std::string>();
        // 计算纹理绝对路径
        auto texture_path = resolvePath(image_path, file_path);
        // 创建精灵
        auto tile_info = TileInfo{Sprite{texture_path, texture_rect}, getTileTypeById(tileset, local_id)};

        for (const auto& tile_json : tileset["tiles"]) {
            auto tile_id = tile_json.value("id", -1);
            if (tile_id == local_id) {
                // 补充动画信息 （瓦片动画为animation字段，且必须为数组，目前只考虑单一图片情况）
                if (tile_json.contains("animation") && tile_json["animation"].is_array()) {
                    std::vector<AnimationFrame> animation_frames;
                    for (const auto& frame : tile_json["animation"]) {
                        // 每个瓦片动画帧json有两个信息：tileid 和 duration
                        auto duration = std::chrono::duration<double, std::milli>(frame.value("duration", 100.0f));
                        int id = frame.value("tileid", 0);
                        auto frame_rect = getTextureRect(tileset, id);  // 根据id获取纹理源矩形
                        // 源矩形 + 时长，组成一个动画帧
                        animation_frames.emplace_back(frame_rect, duration);
                    }
                    tile_info.animation_ = Animation{std::move(animation_frames)};
                }
                // 补充属性信息
                if (tile_json.contains("properties")) {
                    tile_info.properties_ = tile_json["properties"];
                }
            }
        }

        return tile_info;
    } else {  // 这是多图片的情况
        if (!tileset.contains("tiles") ||
            !tileset["tiles"].is_array()) {  // 没有tiles字段的话不符合数据格式要求，直接返回空的瓦片信息
            spdlog::error("Tileset 文件 '{}' 缺少 'tiles' 属性。", tileset_it->first);
            return std::nullopt;
        }
        // 遍历tiles数组，根据id查找对应的瓦片
        for (const auto& tile_json : tileset["tiles"]) {
            auto tile_id = tile_json.value("id", -1);
            if (tile_id == local_id) {
                if (!tile_json.contains("image")) {  //
                                                     // 没有image字段的话不符合数据格式要求，直接返回空的瓦片信息
                    spdlog::error("Tileset 文件 '{}' 中瓦片 {} 缺少 'image' 属性。", tileset_it->first, tile_id);
                    return std::nullopt;
                }
                // --- 接下来根据必要信息创建并返回 TileInfo ---
                // 获取图片路径
                auto texture_path = resolvePath(tile_json["image"].get<std::string>(), file_path);
                // 先确认图片尺寸
                auto image_width = tile_json.value("imagewidth", 0);
                auto image_height = tile_json.value("imageheight", 0);
                // 从json中获取源矩形信息
                Rect texture_rect = {
                    // tiled中源矩形信息只有设置了才会有值，没有就是默认值
                    glm::vec2(tile_json.value("x", 0.0f), tile_json.value("y", 0.0f)),
                    glm::vec2(tile_json.value("width", image_width), tile_json.value("height", image_height)),
                };
                auto tile_info = TileInfo{Sprite{texture_path, texture_rect}, getTileType(tile_json)};
                // scene_->getContext().getResourceManager().loadTexture(entt::hashed_string(texture_path.c_str()),
                // texture_path);  // 确保纹理被加载
                // 补充属性信息
                if (tile_json.contains("properties")) {
                    tile_info.properties_ = tile_json["properties"];
                }
                return tile_info;
            }
        }
    }
    // 如果能走到这里，说明查找失败，返回空的瓦片信息
    spdlog::error("图块集 '{}' 中未找到gid为 {} 的瓦片。", tileset_it->first, gid);
    return std::nullopt;
}

// std::optional<nlohmann::json> LevelLoader::getTileJsonByGid(int gid) const {
//     if (gid == 0) {
//         return std::nullopt;
//     }

//     // 1. 查找tileset_data_中键小于等于gid的最近元素
//     auto tileset_it = tileset_data_.upper_bound(gid);
//     if (tileset_it == tileset_data_.begin()) {
//         spdlog::error("gid为 {} 的瓦片未找到图块集。", gid);
//         return std::nullopt;
//     }
//     --tileset_it;
//     // 2. 获取图块集json对象
//     const auto& tileset = tileset_it->second;
//     if (!tileset.contains("tiles")) {  // 没有tiles字段的话不符合数据格式要求，直接返回空
//         spdlog::error("Tileset 文件 '{}' 缺少 'tiles' 属性。", tileset_it->first);
//         return std::nullopt;
//     }
//     auto local_id = gid - tileset_it->first;  // 计算瓦片在图块集中的局部ID
//     // 3. 遍历tiles数组，根据id查找对应的瓦片并返回瓦片json
//     const auto& tiles_json = tileset["tiles"];
//     for (const auto& tile_json : tiles_json) {
//         auto tile_id = tile_json.value("id", 0);
//         if (tile_id == local_id) {  // 找到对应的瓦片，返回瓦片json
//             return tile_json;
//         }
//     }
//     return std::nullopt;
// }

void LevelLoader::loadTileset(std::string_view tileset_path, int first_gid) {
    std::ifstream file(tileset_path.data());
    if (!file.is_open()) {
        spdlog::error("无法打开 Tileset 文件: {}", tileset_path);
        return;
    }

    nlohmann::json ts_json;
    try {
        file >> ts_json;
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("解析 Tileset JSON 文件 '{}' 失败: {} (at byte {})", tileset_path, e.what(), e.byte);
        return;
    }
    ts_json["file_path"] = tileset_path;  // 将文件路径存储到json中，后续解析图片路径时需要
    tileset_data_[first_gid] = ts_json;
    spdlog::info("Tileset 文件 '{}' 加载完成, firstgid: {}", tileset_path, first_gid);
}

std::string LevelLoader::resolvePath(std::string_view relative_path, std::string_view file_path) const {
    try {
        // 获取地图文件的父目录（相对于可执行文件）
        auto map_dir = std::filesystem::path(file_path).parent_path();
        // 合并路径（相对于可执行文件）并返回。
        return std::filesystem::canonical(map_dir / relative_path);
    } catch (const std::exception& e) {
        spdlog::error("解析路径失败: {}", e.what());
        return std::string(relative_path);
    }
}

}  // namespace pyc::monster_war