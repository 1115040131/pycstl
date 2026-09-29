module;

#include <string>

#include <glm/glm.hpp>

export module ghost_escape.screen.hud_skill;

export import ghost_escape.affiliate.sprite;
export import ghost_escape.core;

export namespace pyc::sdl3 {

class HUDSkill : public ObjectScreen {
public:
    static HUDSkill* CreateAndSet(Object* parent, const std::string& file_path, const glm::vec2& render_position,
                                  float scale = 1.0f, Anchor anchor = Anchor::kCenter);

    virtual void render() override;

    void setPercent(float percent);

protected:
    Sprite* icon_{};
};

}  // namespace pyc::sdl3
