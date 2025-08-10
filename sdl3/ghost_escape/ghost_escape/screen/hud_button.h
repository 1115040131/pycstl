#pragma once

#include "ghost_escape/affiliate/sprite.h"
#include "ghost_escape/core/object_screen.h"

namespace pyc {
namespace sdl3 {

class HUDButton : public ObjectScreen {
public:
    static HUDButton* CreateAndSet(Object* parent, const glm::vec2 render_position, const std::string& file_normal,
                                   const std::string& file_hover, const std::string& file_press,
                                   float scale = 1.0f, Anchor anchor = Anchor::kCenter);

    virtual bool handleEvents(const SDL_Event& event) override;

    virtual void update(std::chrono::duration<float> delta) override;

    bool getIsTrigger();

    void setScale(float scale) {
        sprite_normal_->setScale(scale);
        sprite_hover_->setScale(scale);
        sprite_press_->setScale(scale);
    }

private:
    void checkHover();

    void checkState();

protected:
    Sprite* sprite_normal_{};
    Sprite* sprite_hover_{};
    Sprite* sprite_press_{};

    bool is_hover_{};
    bool is_press_{};
    bool is_trigger_{};
};

}  // namespace sdl3
}  // namespace pyc