module;

#include <chrono>
#include <memory>
#include <string>

#include <glm/glm.hpp>

export module ghost_escape.world.spell;

export import ghost_escape.core;
export import ghost_escape.affiliate.sprite_anim;

export namespace pyc::sdl3 {

class Spell : public ObjectWorld {
public:
    static std::unique_ptr<Spell> Create(const std::string& file_path, const glm::vec2& position, float damage,
                                         float scale = 1.F, Anchor = Anchor::kCenter);

    static Spell* CreateAndSet(Object* parent, const std::string& file_path, const glm::vec2& position,
                               float damage, float scale = 1.F, Anchor = Anchor::kCenter);

    virtual void update(std::chrono::duration<float> delta) override;

private:
    void attack();

protected:
    SpriteAnim* sprite_anim_{};
    float damage_{60.F};
};

}  // namespace pyc::sdl3
