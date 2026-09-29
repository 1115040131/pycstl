module;

#include <chrono>
#include <memory>

#include <glm/glm.hpp>

export module ghost_escape.enemy;

export import ghost_escape.actor;
import ghost_escape.affiliate.sprite_anim;

export namespace pyc::sdl3 {

class Enemy : public Actor {
public:
    static std::unique_ptr<Enemy> Create(const glm::vec2& position, Actor* target);

    virtual void init() override;

    virtual void update(std::chrono::duration<float> delta) override;

    void setTarget(Actor* target) { target_ = target; }

private:
    enum class State {
        kNormal,
        kHurt,
        kDie,
    };

    void aimTarget();
    void attack();
    void checkState();
    void changeState(State state);

    void remove();

private:
    State state_{State::kNormal};

    Actor* target_{};

    SpriteAnim* anim_normal_{};
    SpriteAnim* anim_hurt_{};
    SpriteAnim* anim_die_{};

    SpriteAnim* current_anim_{};

    int score_{10};
};

}  // namespace pyc::sdl3
