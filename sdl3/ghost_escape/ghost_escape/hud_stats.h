#pragma once

#include "sdl3/common/affiliate/sprite.h"
#include "sdl3/common/core/actor.h"
#include "sdl3/common/core/object_screen.h"

namespace pyc {
namespace sdl3 {

class HUDStatus : public ObjectScreen {
public:
    static HUDStatus* CreateAndSet(Object* parent, Actor* target, glm::vec2 render_position);

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