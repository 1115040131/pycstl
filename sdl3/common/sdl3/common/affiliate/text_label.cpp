#include "sdl3/common/affiliate/text_label.h"

namespace pyc {
namespace sdl3 {

TextLabel* TextLabel::CreateAndSet(ObjectScreen* parent, std::string_view text, const std::string& font_path,
                                   int font_size, Anchor anchor) {
    auto text_label = std::make_unique<TextLabel>();
    text_label->init();
#ifdef DEBUG_MODE
    text_label->SET_NAME(TextLabel);
#endif
    text_label->setFont(font_path, font_size);
    text_label->setText(text);
    text_label->anchor_ = anchor;
    text_label->updateSize();
    return static_cast<TextLabel*>(parent->addChild(std::move(text_label)));
}

void TextLabel::clean() {
    TTF_DestroyText(ttf_text_);
    ObjectAffiliate::clean();
}

void TextLabel::render() {
    auto render_position = getRenderPosition();
    TTF_DrawRendererText(ttf_text_, render_position.x, render_position.y);
}

void TextLabel::setFont(const std::string& font_path, int font_size) {
    font_path_ = font_path;
    font_size_ = font_size;
    refreshTextFont();
}

void TextLabel::setFontPath(const std::string& font_path) {
    font_path_ = font_path;
    refreshTextFont();
}

void TextLabel::setFontSize(int font_size) {
    font_size_ = font_size;
    refreshTextFont();
}

void TextLabel::refreshTextFont() {
    if (!ttf_text_) {
        ttf_text_ = game_.createTTF_Text("", font_path_, font_size_);
    }
    auto font = game_.getAssetStore()->getFont(font_path_, font_size_);
    TTF_SetTextFont(ttf_text_, font);
}

void TextLabel::updateSize() {
    int w;
    int h;
    TTF_GetTextSize(ttf_text_, &w, &h);
    setSize(glm::vec2{
        static_cast<float>(w),
        static_cast<float>(h),
    });
}

}  // namespace sdl3
}  // namespace pyc
