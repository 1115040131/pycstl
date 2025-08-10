#pragma once

#include "ghost_escape/core/object_affiliate.h"

namespace pyc {
namespace sdl3 {

class TextLabel : public ObjectAffiliate {
public:
    static TextLabel* CreateAndSet(ObjectScreen* parent, std::string_view text, const std::string& font_path,
                                   int font_size, Anchor anchor = Anchor::kCenter);

    virtual void clean() override;

    virtual void render() override;

    void setFont(const std::string& font_path, int font_size);

    void setFontPath(const std::string& font_path);

    void setFontSize(int font_size);

    std::string_view getText() const { return ttf_text_ ? ttf_text_->text : std::string_view{}; }

    void setText(std::string_view text) {
        if (ttf_text_) {
            TTF_SetTextString(ttf_text_, text.data(), text.size());
        }
    }

private:
    void refreshTextFont();

    void updateSize();

protected:
    TTF_Text* ttf_text_{};
    std::string font_path_;
    int font_size_{16};
};

}  // namespace sdl3
}  // namespace pyc