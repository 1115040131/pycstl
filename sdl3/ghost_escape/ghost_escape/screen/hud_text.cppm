module;

#include <string>
#include <string_view>

#include <glm/glm.hpp>

export module ghost_escape.screen.hud_text;

export import ghost_escape.affiliate.sprite;
export import ghost_escape.affiliate.text_label;
export import ghost_escape.core;

export namespace pyc::sdl3 {

class HUDText : public ObjectScreen {
public:
    static HUDText* CreateAndSet(Object* parent, std::string_view text, const glm::vec2& render_position,
                                 const glm::vec2& size, const std::string& font_path, int font_size,
                                 const std::string& file_path, Anchor anchor = Anchor::kCenter);

    void setText(std::string_view text);

    void setSize(const glm::vec2& size);

    void setSizeByText(float margin = 50.0f);

    void setBackground(const std::string& file_path);

protected:
    TextLabel* text_label_{};
    Sprite* sprite_bg_{};
    glm::vec2 size_{};  // 背景图大小
};

}  // namespace pyc::sdl3
