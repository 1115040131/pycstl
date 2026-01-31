#pragma once

#include <optional>
#include <string>

#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>

#include "monster_war/engine/utils/math.h"

namespace pyc::monster_war {

/**
 * @brief 表示要绘制的视觉精灵的数据。
 *
 * 包含纹理标识符、要绘制的纹理部分（源矩形）以及翻转状态。
 * 位置、缩放和旋转由外部（例如 UIImage）标识。
 * 渲染工作由 Renderer 类完成。（传入Image作为参数）
 */
class Image final {
public:
    /**
     * @brief 默认构造函数（创建一个空的/无效的精灵）
     */
    explicit Image() = default;

    /**
     * @brief 构造一个精灵 （通过纹理路径构造）
     *
     * @param texture_path 纹理资源的文件路径。不应为空。
     * @param source_rect 可选的源矩形（SDL_FRect），定义要使用的纹理部分。如果为 std::nullopt，则使用整个纹理。
     * @param is_flipped 是否水平翻转
     */
    explicit Image(std::string_view texture_path, std::optional<Rect> source_rect = std::nullopt,
                   bool is_flipped = false)
        : texture_path_(texture_path),
          texture_id_(entt::hashed_string(texture_path.data())),
          source_rect_(std::move(source_rect)),
          is_flipped_(is_flipped) {}

    /**
     * @brief 构造一个精灵 （通过纹理ID构造）
     *
     * @param texture_id 纹理资源的标识符。不应为空。
     * @param source_rect 可选的源矩形（SDL_FRect），定义要使用的纹理部分。如果为 std::nullopt，则使用整个纹理。
     * @param is_flipped 是否水平翻转
     * @note 用此方法，需确保对应ID的纹理已经加载到ResourceManager中，因此不需要再提供纹理路径。
     */
    explicit Image(entt::id_type texture_id, std::optional<Rect> source_rect = std::nullopt,
                   bool is_flipped = false)
        : texture_id_(texture_id), source_rect_(std::move(source_rect)), is_flipped_(is_flipped) {}

    // --- getters and setters ---
    std::string_view getTexturePath() const { return texture_path_; }          ///< @brief 获取纹理路径
    entt::id_type getTextureId() const { return texture_id_; }                 ///< @brief 获取纹理 ID
    const std::optional<Rect>& getSourceRect() const { return source_rect_; }  ///< @brief 获取源矩形
    bool isFlipped() const { return is_flipped_; }                             ///< @brief 获取是否水平翻转

    /**
     * @brief 设置纹理路径同时更新纹理ID
     * @param texture_path 纹理资源的文件路径。不应为空。
     */
    void setTexture(std::string_view texture_path) {
        texture_path_ = texture_path.data();
        texture_id_ = entt::hashed_string(texture_path.data());
    }

    ///< @brief 设置纹理ID (需确保已载入)
    void setTextureId(entt::id_type texture_id) { texture_id_ = texture_id; }

    /**
     * @brief 设置源矩形 (如果使用整个纹理则为 std::nullopt)
     * @param source_rect 源矩形。如果使用整个纹理则为 std::nullopt
     */
    void setSourceRect(std::optional<Rect> source_rect) { source_rect_ = std::move(source_rect); }

    /**
     * @brief 设置是否水平翻转
     * @param flipped 是否水平翻转
     */
    void setFlipped(bool flipped) { is_flipped_ = flipped; }

private:
    std::string texture_path_;  ///< @brief 纹理资源的文件路径
    entt::id_type texture_id_{
        entt::null};                   ///< @brief 纹理资源的标识符 (entt::null是推荐的初始化方式，表示无效的ID)
    std::optional<Rect> source_rect_;  ///< @brief 可选：要绘制的纹理部分
    bool is_flipped_ = false;          ///< @brief 是否水平翻转
};

}  // namespace pyc::monster_war
