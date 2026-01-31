#include "monster_war/engine/ui/ui_label.h"

#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/render/text_renderer.h"

namespace pyc::monster_war {

UILabel::UILabel(TextRenderer& text_renderer, std::string_view text, std::string_view font_path, int font_size,
                 FColor text_color, glm::vec2 position)
    : UIElement(std::move(position)),
      text_renderer_(text_renderer),
      text_(text),
      font_path_(font_path),
      font_id_(entt::hashed_string(font_path.data())),
      font_size_(font_size),
      text_fcolor_(std::move(text_color)) {
    // 获取文本渲染尺寸 (函数内部会确保字体资源被加载)
    size_ = text_renderer_.getTextSize(text_, font_id_, font_size_, font_path_);
    spdlog::trace("UILabel 构造完成");
}

void UILabel::render(Context& context) {
    if (!visible_ || text_.empty()) {
        return;
    }

    text_renderer_.drawUIText(text_, font_id_, font_size_, getScreenPosition(), text_fcolor_);

    // 渲染子元素（调用基类方法）
    UIElement::render(context);
}

void UILabel::setText(std::string_view text) {
    text_ = text;
    size_ = text_renderer_.getTextSize(text_, font_id_, font_size_, font_path_);
}

void UILabel::setFontPath(std::string_view font_path) {
    font_path_ = font_path;
    font_id_ = entt::hashed_string(font_path.data());
    size_ = text_renderer_.getTextSize(text_, font_id_, font_size_, font_path_);
}

void UILabel::setFontSize(int font_size) {
    font_size_ = font_size;
    size_ = text_renderer_.getTextSize(text_, font_id_, font_size_, font_path_);
}

}  // namespace pyc::monster_war