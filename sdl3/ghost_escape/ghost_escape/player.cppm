module;

#include <chrono>

export module ghost_escape.player;

export import ghost_escape.actor;
import ghost_escape.affiliate.sprite_anim;
import ghost_escape.raw.timer;
import ghost_escape.weapon_thunder;
import ghost_escape.world.effect;

export namespace pyc::sdl3 {

class Player : public Actor {
public:
    virtual void init() override;
    virtual void clean() override;

    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

    virtual void takeDamage(double damage) override;

private:
    void keyboardControl();
    void syncCamera();
    void checkState();
    void checkIsDead();

private:
    SpriteAnim* anim_idle_{};
    SpriteAnim* anim_move_{};
    Effect* effect_{};
    WeaponThunder* weapon_thunder_{};
    Timer* flash_timer_{};
    bool is_moving_{};
};

}  // namespace pyc::sdl3
