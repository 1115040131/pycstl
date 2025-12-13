#pragma once

#include "monster_war/engine/ui/ui_element.h"

#include "monster_war/engine/render/sprite.h"

namespace pyc::monster_war {

/**
 * @brief 一个用于显示纹理或部分纹理的UI元素。
 *
 * 继承自UIElement并添加了渲染图像的功能。
 */
class UIImage final : public UIElement {
public:
    /**
     * @brief 构造一个UIImage对象。（通过纹理路径构造）
     *
     * @param texture_path 要显示的纹理路径。
     * @param position 图像的局部位置。
     * @param size 图像元素的大小。（如果为{0,0}，则使用纹理的原始尺寸）
     * @param source_rect 可选：要绘制的纹理部分。（如果为空，则使用纹理的整个区域）
     * @param is_flipped 可选：精灵是否应该水平翻转。
     */
    UIImage(std::string_view texture_path, glm::vec2 position = {0.0f, 0.0f}, glm::vec2 size = {0.0f, 0.0f},
            std::optional<Rect> source_rect = std::nullopt, bool is_flipped = false);

    /**
     * @brief 构造一个UIImage对象。（通过纹理ID构造）
     *
     * @param texture_id 要显示的纹理ID。
     * @param position 图像的局部位置。
     * @param size 图像元素的大小。（如果为{0,0}，则使用纹理的原始尺寸）
     * @param source_rect 可选：要绘制的纹理部分。（如果为空，则使用纹理的整个区域）
     * @param is_flipped 可选：精灵是否应该水平翻转。
     * @note 用此方法，需确保对应ID的纹理已经加载到ResourceManager中，因此不需要再提供纹理路径。
     */
    UIImage(entt::id_type texture_id, glm::vec2 position = {0.0f, 0.0f}, glm::vec2 size = {0.0f, 0.0f},
            std::optional<Rect> source_rect = std::nullopt, bool is_flipped = false);

    /**
     * @brief 构造一个UIImage对象。（通过Sprite对象构造）
     *
     * @param sprite 要显示的Sprite对象。
     * @param position 图像的局部位置。
     * @param size 图像元素的大小。（如果为{0,0}，则使用纹理的原始尺寸）
     */
    UIImage(const Sprite& sprite, glm::vec2 position = {0.0f, 0.0f}, glm::vec2 size = {0.0f, 0.0f});

    // --- 核心方法 ---
    void render(Context& context) override;

    // --- Setters & Getters ---
    const Sprite& getSprite() const { return sprite_; }
    void setSprite(Sprite sprite) { sprite_ = std::move(sprite); }

    std::string_view getTexturePath() const { return sprite_.getTexturePath(); }
    entt::id_type getTextureId() const { return sprite_.getTextureId(); }
    void setTexture(std::string_view texture_id) { sprite_.setTexture(texture_id); }

    const std::optional<Rect>& getSourceRect() const { return sprite_.getSourceRect(); }
    void setSourceRect(std::optional<Rect> source_rect) { sprite_.setSourceRect(std::move(source_rect)); }

    bool isFlipped() const { return sprite_.isFlipped(); }
    void setFlipped(bool flipped) { sprite_.setFlipped(flipped); }

private:
    Sprite sprite_;
};

}  // namespace pyc::monster_war