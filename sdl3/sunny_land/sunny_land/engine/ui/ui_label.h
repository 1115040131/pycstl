#pragma once

#include "sunny_land/engine/ui/ui_element.h"

namespace pyc::sunny_land {

class TextRenderer;

/**
 * @brief UILabel 类用于创建和管理用户界面中的文本标签
 *
 * UILabel 继承自 UIElement，提供了文本渲染功能。
 * 它可以设置文本内容、字体ID、字体大小和文本颜色。
 *
 * @note 需要一个文本渲染器来获取和更新文本尺寸。
 */
class UILabel final : public UIElement {
public:
    /**
     * @brief 构造一个UILabel
     *
     * @param text_renderer 文本渲染器
     * @param text 文本内容
     * @param font_id 字体ID
     * @param font_size 字体大小
     * @param text_color 文本颜色
     */
    UILabel(TextRenderer& text_renderer, std::string_view text, std::string_view font_id, int font_size = 16,
            FColor text_color = {1.0f, 1.0f, 1.0f, 1.0f}, glm::vec2 position = {0.0f, 0.0f});

    // --- 核心方法 ---
    void render(Context& context) override;

    // --- Setters & Getters ---
    std::string_view getText() const { return text_; }
    std::string_view getFontId() const { return font_id_; }
    int getFontSize() const { return font_size_; }
    const FColor& getTextFColor() const { return text_fcolor_; }

    void setText(std::string_view text);       ///< @brief 设置文本内容, 同时更新尺寸
    void setFontId(std::string_view font_id);  ///< @brief 设置字体ID, 同时更新尺寸
    void setFontSize(int font_size);           ///< @brief 设置字体大小, 同时更新尺寸
    void setTextFColor(FColor text_fcolor) { text_fcolor_ = std::move(text_fcolor); }  ///< @brief 设置文本颜色

private:
    TextRenderer& text_renderer_;  ///< @brief 需要文本渲染器，用于获取/更新文本尺寸

    std::string text_;     ///< @brief 文本内容
    std::string font_id_;  ///< @brief 字体ID
    int font_size_;        ///< @brief 字体大小
    FColor text_fcolor_ = {1.0f, 1.0f, 1.0f, 1.0f};
    /* 可添加其他内容，例如边框、底色 */
};

}  // namespace pyc::sunny_land