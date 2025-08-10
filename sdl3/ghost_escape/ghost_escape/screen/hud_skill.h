#pragma once

#include "ghost_escape/affiliate/sprite.h"
#include "ghost_escape/core/object_screen.h"

namespace pyc {
namespace sdl3 {

class HUDSkill : public ObjectScreen {
public:
    static HUDSkill* CreateAndSet(Object* parent, const std::string& file_path, const glm::vec2& render_position,
                                  float scale = 1.0f, Anchor anchor = Anchor::kCenter);

    virtual void render() override;

    void setPercent(float percent);

protected:
    Sprite* icon_{};
};

}  // namespace sdl3
}  // namespace pyc