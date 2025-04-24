#pragma once

#include "sdl3/common/affiliate/sprite_anim.h"
#include "sdl3/common/core/object_world.h"

namespace pyc {
namespace sdl3 {

class Effect : public ObjectWorld {
public:
    static Effect* CreateAndSet(Object* parent, const std::string& file_path, const glm::vec2& position,
                                float scale = 1.F, std::unique_ptr<ObjectWorld> next = nullptr);

    virtual void clean() override;

    virtual void update(std::chrono::duration<float> delta) override;

    void setNext(std::unique_ptr<ObjectWorld> next) { next_ = std::move(next); }

private:
    void checkFinish();

private:
    SpriteAnim* sprite_{};
    std::unique_ptr<ObjectWorld> next_{};
};

}  // namespace sdl3
}  // namespace pyc
