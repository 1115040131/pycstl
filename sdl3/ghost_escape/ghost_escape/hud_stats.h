#pragma once

#include "ghost_escape/affiliate/sprite.h"
#include "ghost_escape/core/actor.h"
#include "ghost_escape/core/object_screen.h"

namespace pyc {
namespace sdl3 {

class HUDStatus : public ObjectScreen {
public:
    static HUDStatus* CreateAndSet(Object* parent, Actor* target, const glm::vec2& render_position);

    virtual void init() override;

    virtual void update(std::chrono::duration<float> delta) override;

protected:
    Actor* target_{};

    Sprite* health_bar_{};
    Sprite* health_bar_bg_{};
    Sprite* health_icon_{};
    Sprite* mana_bar_{};
    Sprite* mana_bar_bg_{};
    Sprite* mana_icon_{};
};

}  // namespace sdl3
}  // namespace pyc