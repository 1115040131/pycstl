module;

#include <chrono>

#include <glm/glm.hpp>

export module ghost_escape.hud_stats;

export import ghost_escape.actor;
export import ghost_escape.affiliate.sprite;
export import ghost_escape.core;

export namespace pyc::sdl3 {

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

}  // namespace pyc::sdl3
