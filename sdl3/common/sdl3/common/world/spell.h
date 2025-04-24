#pragma once

#include "sdl3/common/affiliate/sprite_anim.h"
#include "sdl3/common/core/object_world.h"

namespace pyc {
namespace sdl3 {

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

}  // namespace sdl3
}  // namespace pyc