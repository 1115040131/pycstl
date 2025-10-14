#include "sunny_land/engine/scene/level_loader.h"

#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/animation_component.h"
#include "sunny_land/engine/component/audio_component.h"
#include "sunny_land/engine/component/collider_component.h"
#include "sunny_land/engine/component/health_component.h"
#include "sunny_land/engine/component/parallax_component.h"
#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/component/tilelayer_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/render/animation.h"
#include "sunny_land/engine/scene/scene.h"

namespace pyc::sunny_land {

bool LevelLoader::loadLevel(std::string_view level_path, Scene& scene) {
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
    auto texture_id = resolvePath(image_path, map_path_);

    // 获取图层偏移量，视差因子及重复标志
    auto offset = glm::vec2(layer_json.value("offsetx", 0.0f), layer_json.value("offsety", 0.0f));
    auto scroll_factor = glm::vec2(layer_json.value("parallaxx", 1.0f), layer_json.value("parallaxy", 1.0f));
    auto repeat = glm::vec2(layer_json.value("repeatx", false), layer_json.value("repeaty", false));

    // 获取图层名称
    const auto& layer_name = layer_json.value("name", "Unnamed");

    // 创建游戏对象
    auto game_object = std::make_unique<GameObject>(layer_name);
    game_object->addComponent<TransformComponent>(offset);
    game_object->addComponent<ParallaxComponent>(texture_id, scroll_factor, repeat);

    // 添加到场景中
    scene.addGameObject(std::move(game_object));
    spdlog::info("加载图层: '{}' 完成", layer_name);
}

void LevelLoader::loadTileLayer(const nlohmann::json& layer_json, Scene& scene) {
    if (!layer_json.contains("data") || !layer_json["data"].is_array()) {
        spdlog::error("图层 '{}' 缺少 'data' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }
    // 准备 TileInfo Vector (瓦片数量 = 地图宽度 * 地图高度)
    std::vector<TileInfo> tiles;
    tiles.reserve(map_size_.x * map_size_.y);

    // 根据gid获取必要信息，并依次填充 TileInfo Vector
    for (const auto& gid : layer_json["data"]) {
        tiles.push_back(getTileInfoByGid(gid));
    }

    // 获取图层名称
    const auto& layer_name = layer_json.value("name", "Unnamed");

    // 创建游戏对象
    auto game_object = std::make_unique<GameObject>(layer_name);
    game_object->addComponent<TileLayerComponent>(tile_size_, map_size_, std::move(tiles));

    // 添加到场景中
    scene.addGameObject(std::move(game_object));
    spdlog::info("加载图层: '{}' 完成", layer_name);
}

void LevelLoader::loadObjectLayer(const nlohmann::json& layer_json, Scene& scene) {
    if (!layer_json.contains("objects") || !layer_json["objects"].is_array()) {
        spdlog::error("图层 '{}' 缺少 'objects' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }
    for (const auto& object_json : layer_json["objects"]) {
        auto gid = object_json.value("gid", 0);
        if (gid == 0) {  // gid 为 0 即不存在, 代表需要自己绘制
            // TODO
        } else {
            auto tile_info = getTileInfoByGid(gid);
            if (tile_info.sprite.getTextureId().empty()) {
                spdlog::error("gid为 {} 的瓦片没有图像纹理。", gid);
                continue;
            }
            // 获取Transform相关信息
            auto position = glm::vec2(object_json.value("x", 0.0f), object_json.value("y", 0.0f));
            auto dst_size = glm::vec2(object_json.value("width", 0.0f), object_json.value("height", 0.0f));
            position.y -= dst_size.y;  // 实际position需要进行调整(左下角到左上角)

            auto rotation = object_json.value("rotation", 0.0f);
            auto src_size_opt = tile_info.sprite.getSourceRect();
            if (!src_size_opt) {  // 正常情况下，所有瓦片的Sprite都设置了源矩形，没有代表某处出错
                spdlog::error("gid为 {} 的瓦片没有源矩形。", gid);
                continue;
            }
            auto src_size = glm::vec2(src_size_opt->w, src_size_opt->h);
            auto scale = dst_size / src_size;

            // 获取图层名称
            const auto& object_name = object_json.value("name", "Unnamed");

            // 创建游戏对象
            auto game_object = std::make_unique<GameObject>(object_name);
            game_object->addComponent<TransformComponent>(position, scale, rotation);
            game_object->addComponent<SpriteComponent>(std::move(tile_info.sprite),
                                                       scene.getContext().getResourceManager());

            // 获取瓦片json信息
            // 1. 必然存在，因为getTileInfoByGid(gid)函数已经顺利执行
            // 2. 这里再获取json，实际上检索了两次，未来可以优化
            auto tile_json = getTileJsonByGid(gid);

            // 获取碰信息：如果是SOLID类型，则添加物理组件，且图片源矩形区域就是碰撞盒大小
            if (tile_info.type == TileType::SOLID) {
                auto collider = std::make_unique<AABBCollider>(src_size);
                game_object->addComponent<ColliderComponent>(std::move(collider));
                game_object->addComponent<PhysicsComponent>(&scene.getContext().getPhysicsEngine(), false);
                // 设置标签方便物理引擎检索
                game_object->setTag("solid");
            } else if (auto rect = getColliderRect(tile_json)) {  // 如果非SOLID类型，检查自定义碰撞盒是否存在
                // 如果有，添加碰撞组件
                auto collider = std::make_unique<AABBCollider>(rect->size);
                auto cc = game_object->addComponent<ColliderComponent>(std::move(collider));
                cc->setOffset(rect->position);  // 自定义碰撞盒的坐标是相对于图片坐标，也就是针对Transform的偏移量

                // 获取重力信息并设置物理组件
                auto gravity = getTileProperty<bool>(tile_json, "gravity");
                game_object->addComponent<PhysicsComponent>(&scene.getContext().getPhysicsEngine(),
                                                            gravity && gravity.value());
            }

            // 获取标签信息并设置
            if (auto tag = getTileProperty<std::string>(tile_json, "tag")) {
                game_object->setTag(tag.value());
            } else if (tile_info.type ==
                       TileType::HAZARD) {  // 如果是危险瓦片，且没有手动设置标签，则自动设置标签为 "hazard"
                game_object->setTag("hazard");
            }

            // 获取动画信息并设置
            if (auto animation = getTileProperty<std::string>(tile_json, "animation")) {
                // 解析 string 为 JSON 对象
                nlohmann::json anim_json;
                try {
                    anim_json = nlohmann::json::parse(animation->data());
                } catch (const nlohmann::json::parse_error& e) {
                    spdlog::error("解析动画 JSON 字符串失败: {}", e.what());
                    continue;  // 跳过此对象
                }
                // 添加AnimationComponent
                auto* animation_component = game_object->addComponent<AnimationComponent>();
                // 添加动画到 AnimationComponent
                addAnimation(anim_json, animation_component, src_size);
            }

            // 获取音效信息并设置
            if (auto sound_string = getTileProperty<std::string>(tile_json, "sound")) {
                // 解析string为JSON对象
                nlohmann::json sound_json;
                try {
                    sound_json = nlohmann::json::parse(sound_string.value());
                } catch (const nlohmann::json::parse_error& e) {
                    spdlog::error("解析音效 JSON 字符串失败: {}", e.what());
                    continue;  // 跳过此对象
                }
                // 添加AudioComponent
                auto* audio_component = game_object->addComponent<AudioComponent>(
                    &scene.getContext().getAudioPlayer(), &scene.getContext().getCamera());
                // 添加音效到 AudioComponent
                addSound(sound_json, audio_component);
            }

            // 获取生命值信息并设置
            if (auto health = getTileProperty<int>(tile_json, "health")) {
                // 添加 HealthComponent
                game_object->addComponent<HealthComponent>(health.value());
            }

            // 添加到场景中
            scene.addGameObject(std::move(game_object));
            spdlog::info("加载图层: '{}' 完成", object_name);
        }
    }
}

void LevelLoader::addAnimation(const nlohmann::json& anim_json, AnimationComponent* animation_component,
                               const glm::vec2& sprite_size) {
    // 检查 anim_json 必须是一个对象，并且 ac 不能为 nullptr
    if (!anim_json.is_object() || !animation_component) {
        spdlog::error("无效的动画 JSON 或 AnimationComponent 指针。");
        return;
    }
    // 遍历动画 JSON 对象中的每个键值对（动画名称 : 动画信息）
    for (const auto& anim : anim_json.items()) {
        std::string_view anim_name = anim.key();
        const auto& anim_info = anim.value();
        if (!anim_info.is_object()) {
            spdlog::warn("动画 '{}' 的信息无效或为空。", anim_name);
            continue;
        }
        // 获取可能存在的动画帧信息
        auto duration_ms = std::chrono::milliseconds(anim_info.value("duration", 100));  // 默认持续时间为100毫秒
        auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(duration_ms);  // 转换为秒
        auto row = anim_info.value("row", 0);                                                   // 默认行数为0
        // 帧信息（数组）是必须存在的
        if (!anim_info.contains("frames") || !anim_info["frames"].is_array()) {
            spdlog::warn("动画 '{}' 缺少 'frames' 数组。", anim_name);
            continue;
        }
        // 创建一个Animation对象 (默认为循环播放)
        auto animation = std::make_unique<Animation>(anim_name);

        // 遍历数组并进行添加帧信息到animation对象
        for (const auto& frame : anim_info["frames"]) {
            if (!frame.is_number_integer()) {
                spdlog::warn("动画 {} 中 frames 数组格式错误！", anim_name);
                continue;
            }
            auto column = frame.get<int>();
            // 计算源矩形
            SDL_FRect src_rect = {
                column * sprite_size.x,
                row * sprite_size.y,
                sprite_size.x,
                sprite_size.y,
            };
            // 添加动画帧到 Animation
            animation->addFrame(src_rect, duration);
        }
        // 将 Animation 对象添加到 AnimationComponent 中
        animation_component->addAnimation(std::move(animation));
    }
}

void LevelLoader::addSound(const nlohmann::json& sound_json, AudioComponent* audio_component) {
    if (!sound_json.is_object() || !audio_component) {
        spdlog::error("无效的音效 JSON 或 AudioComponent 指针。");
        return;
    }
    // 遍历音效 JSON 对象中的每个键值对（音效id : 音效路径）
    for (const auto& sound : sound_json.items()) {
        const std::string& sound_id = sound.key();
        const std::string& sound_path = sound.value();
        if (sound_id.empty() || sound_path.empty()) {
            spdlog::warn("音效 '{}' 缺少必要信息。", sound_id);
            continue;
        }
        // 添加音效到 AudioComponent
        audio_component->addSound(sound_id, resolvePath(sound_path, map_path_));
    }
}

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

TileType LevelLoader::getTileType(const nlohmann::json& tile_json) const {
    if (tile_json.contains("properties")) {
        for (const auto& property : tile_json["properties"]) {
            auto name = property.value("name", "");
            if (name == "solid") {
                return property.value("value", false) ? TileType::SOLID : TileType::NORMAL;
            } else if (name == "unisolid") {
                return property.value("value", false) ? TileType::UNISOLID : TileType::NORMAL;
            } else if (name == "slope") {
                auto slope_type = property.value("value", "");
                if (slope_type == "0_1") {
                    return TileType::SLOPE_0_1;
                } else if (slope_type == "1_0") {
                    return TileType::SLOPE_1_0;
                } else if (slope_type == "0_2") {
                    return TileType::SLOPE_0_2;
                } else if (slope_type == "2_1") {
                    return TileType::SLOPE_2_1;
                } else if (slope_type == "1_2") {
                    return TileType::SLOPE_1_2;
                } else if (slope_type == "2_0") {
                    return TileType::SLOPE_2_0;
                } else {
                    spdlog::error("未知的斜坡类型: {}", slope_type);
                    return TileType::NORMAL;
                }
            } else if (name == "hazard") {
                return property.value("value", false) ? TileType::HAZARD : TileType::NORMAL;
            } else if (name == "ladder") {
                return property.value("value", false) ? TileType::LADDER : TileType::NORMAL;
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

TileInfo LevelLoader::getTileInfoByGid(int gid) const {
    if (gid == 0) {
        return TileInfo{};
    }

    // upper_bound：查找tileset_data_中键大于 gid 的第一个元素，返回迭代器
    auto tileset_it = tileset_data_.upper_bound(gid);
    if (tileset_it == tileset_data_.begin()) {
        spdlog::error("gid为 {} 的瓦片未找到图块集。", gid);
        return TileInfo{};
    }
    --tileset_it;  // 前移一个位置，这样就得到不大于gid的最近一个元素（我们需要的）

    const auto& tileset = tileset_it->second;
    auto local_id = gid - tileset_it->first;
    const auto& file_path = tileset.value("file_path", "");  // 获取图块集文件路径
    if (file_path.empty()) {
        spdlog::error("Tileset 文件 '{}' 缺少 'file_path' 属性。", tileset_it->first);
        return TileInfo{};
    }
    if (tileset.contains("image")) {  // 这是单一图片的情况
        // 获取图片路径
        auto texture_id = resolvePath(tileset["image"].get<std::string>(), file_path);
        // 计算瓦片在图片网格中的坐标
        auto coordinate_x = local_id % tileset["columns"].get<int>();
        auto coordinate_y = local_id / tileset["columns"].get<int>();
        // 根据坐标确定源矩形
        SDL_FRect texture_rect = {static_cast<float>(coordinate_x * tile_size_.x),
                                  static_cast<float>(coordinate_y * tile_size_.y),
                                  static_cast<float>(tile_size_.x), static_cast<float>(tile_size_.y)};
        auto tile_type = getTileTypeById(tileset, local_id);
        return TileInfo{{texture_id, texture_rect}, tile_type};
    } else {  // 这是多图片的情况
        if (!tileset.contains("tiles") ||
            !tileset["tiles"].is_array()) {  // 没有tiles字段的话不符合数据格式要求，直接返回空的瓦片信息
            spdlog::error("Tileset 文件 '{}' 缺少 'tiles' 属性。", tileset_it->first);
            return TileInfo{};
        }
        for (const auto& tile_json : tileset["tiles"]) {
            auto tile_id = tile_json.value("id", -1);
            if (tile_id == local_id) {
                if (!tile_json.contains("image")) {  // 没有image字段的话不符合数据格式要求，直接返回空的瓦片信息
                    spdlog::error("Tileset 文件 '{}' 中瓦片 {} 缺少 'image' 属性。", tileset_it->first, tile_id);
                    return TileInfo{};
                }
                // --- 接下来根据必要信息创建并返回 TileInfo ---
                // 获取图片路径
                auto texture_id = resolvePath(tile_json["image"].get<std::string>(), file_path);
                // 先确认图片尺寸
                auto image_width = tile_json.value("imagewidth", 0);
                auto image_height = tile_json.value("imageheight", 0);
                // 从json中获取源矩形信息
                SDL_FRect texture_rect = {
                    // tiled中源矩形信息只有设置了才会有值，没有就是默认值
                    static_cast<float>(tile_json.value("x", 0)),
                    static_cast<float>(tile_json.value("y", 0)),
                    static_cast<float>(tile_json.value("width", image_width)),  // 如果未设置，则使用图片尺寸
                    static_cast<float>(tile_json.value("height", image_height)),
                };
                auto tile_type = getTileType(tile_json);
                return TileInfo{{texture_id, texture_rect}, tile_type};
            }
        }
    }
    // 如果能走到这里，说明查找失败，返回空的瓦片信息
    spdlog::error("图块集 '{}' 中未找到gid为 {} 的瓦片。", tileset_it->first, gid);
    return TileInfo{};
}

std::optional<nlohmann::json> LevelLoader::getTileJsonByGid(int gid) const {
    if (gid == 0) {
        return std::nullopt;
    }

    // 1. 查找tileset_data_中键小于等于gid的最近元素
    auto tileset_it = tileset_data_.upper_bound(gid);
    if (tileset_it == tileset_data_.begin()) {
        spdlog::error("gid为 {} 的瓦片未找到图块集。", gid);
        return std::nullopt;
    }
    --tileset_it;
    // 2. 获取图块集json对象
    const auto& tileset = tileset_it->second;
    if (!tileset.contains("tiles")) {  // 没有tiles字段的话不符合数据格式要求，直接返回空
        spdlog::error("Tileset 文件 '{}' 缺少 'tiles' 属性。", tileset_it->first);
        return std::nullopt;
    }
    auto local_id = gid - tileset_it->first;  // 计算瓦片在图块集中的局部ID
    // 3. 遍历tiles数组，根据id查找对应的瓦片并返回瓦片json
    const auto& tiles_json = tileset["tiles"];
    for (const auto& tile_json : tiles_json) {
        auto tile_id = tile_json.value("id", 0);
        if (tile_id == local_id) {  // 找到对应的瓦片，返回瓦片json
            return tile_json;
        }
    }
    return std::nullopt;
}

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
    spdlog::info("Tileset 文件 '{}' 加载完成，firstgid: {}", tileset_path, first_gid);
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

}  // namespace pyc::sunny_land